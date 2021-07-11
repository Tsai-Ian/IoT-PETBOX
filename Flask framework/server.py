from logging import ERROR
from flask import Flask, request, abort, render_template, jsonify, Response,  redirect, url_for, session
from flask_socketio import SocketIO, emit
from flask_mqtt import Mqtt
import json
from flask_mysqldb import MySQL
import MySQLdb.cursors
import re
from datetime import date
from datetime import datetime
from engineio.payload import Payload
import cv2


#Initialize app
app = Flask(__name__)

app.secret_key = "super secret key"
# CONFIG
app.config['MQTT_BROKER_URL'] = 'MQTT HOST'
app.config['SECRET'] = 'my secret key'
app.config['TEMPLATES_AUTO_RELOAD'] = True
app.config['MQTT_BROKER_PORT'] = 1883
app.config['MQTT_USERNAME'] = ''
app.config['MQTT_PASSWORD'] = ''
app.config['MQTT_KEEPALIVE'] = 5
app.config['MQTT_TLS_ENABLED'] = False
app.config['MQTT_CLEAN_SESSION'] = True
app.config['MYSQL_HOST'] = 'MYSQL MASTER HOST'
app.config['MYSQL_USER'] = 'user'
app.config['MYSQL_PASSWORD'] = 'user'
app.config['MYSQL_DB'] = 'pythonlogin'


# Intialize MySQL, MQTT, socketio
mqtt = Mqtt(app)
socketio = SocketIO(app)
mysql = MySQL(app)


class JsonExtendEncoder(json.JSONEncoder):
    """
        This class provide an extension to json serialization for datetime/date.
    """
    def default(self, o):
        """
            provide a interface for datetime/date
        """
        if isinstance(o, datetime):
            return o.strftime('%Y-%m-%d %H:%M:%S')
        elif isinstance(o, date):
            return o.strftime('%Y-%m-%d')
        else:
            return json.JSONEncoder.default(self, o)

class VideoCamera():
    def __init__(self,vurl):
        self.video = cv2.VideoCapture(vurl)

    def __del__(self):
        self.video.release()

    def get_frame(self):
        success, image = self.video.read()
        ret, jpeg = cv2.imencode('.jpg', image)
        return jpeg.tobytes()

def gen(camera):
    while True:
        frame = camera.get_frame()
        yield (b'--frame\r\n'
               b'Content-Type: image/jpeg\r\n\r\n' + frame + b'\r\n\r\n')

#SERVER INDEX

@app.route("/")
def dashboard():
    return render_template('index.html')



# -------------------------- LOGIN START ---------------------------------- #
# http://localhost:5000/pythonlogin/ - this will be the login page, we need to use both GET and POST requests
@app.route('/pythonlogin/', methods=['GET', 'POST'])
def login():
    # Output message if something goes wrong...
    msg = ''
    # Check if "username" and "password" POST requests exist (user submitted form)
    if request.method == 'POST' and 'username' in request.form and 'password' in request.form:
        # Create variables for easy access
        username = request.form['username']
        password = request.form['password']
        # Check if account exists using MySQL
        cursor = mysql.connection.cursor(MySQLdb.cursors.DictCursor)
        cursor.execute('SELECT * FROM accounts WHERE username = %s AND password = %s', (username, password,))
        # Fetch one record and return result
        account = cursor.fetchone()
        cursor.close()

        # If account exists in accounts table in out database
        if account:
            # Create session data, we can access this data in other routes
            session['loggedin'] = True
            session['id'] = account['id']
            session['username'] = account['username']
            # Redirect to home page
            return redirect(url_for('home'))
        else:
            # Account doesnt exist or username/password incorrect
            msg = 'Incorrect username/password!'
    # Show the login form with message (if any)
    return render_template('login.html', msg=msg)
# -------------------------- LOGIN END ------------------------------------ #

# -------------------------- LOGOUT START --------------------------------- #

# http://localhost:5000/python/logout - this will be the logout page
@app.route('/pythonlogin/logout')
def logout():
    # Remove session data, this will log the user out
   session.pop('loggedin', None)
   session.pop('id', None)
   session.pop('username', None)
   # Redirect to login page
   return redirect(url_for('login'))

# -------------------------- LOGOUT END ------------------------------------- #

# -------------------------- REGISTER START --------------------------------- #

# http://localhost:5000/pythinlogin/register - this will be the registration page, we need to use both GET and POST requests
@app.route('/pythonlogin/register', methods=['GET', 'POST'])
def register():
    # Output message if something goes wrong...
    msg = ''
    # Check if "username", "password" and "email" POST requests exist (user submitted form)
    if request.method == 'POST' and 'username' in request.form and 'password' in request.form and 'email' in request.form:
        # Create variables for easy access
        username = request.form['username']
        password = request.form['password']
        email = request.form['email']

                # Check if account exists using MySQL
        cursor = mysql.connection.cursor(MySQLdb.cursors.DictCursor)
        cursor.execute('SELECT * FROM accounts WHERE username = %s', (username,))
        account = cursor.fetchone()
        # If account exists show error and validation checks
        if account:
            msg = 'Account already exists!'
        elif not re.match(r'[^@]+@[^@]+\.[^@]+', email):
            msg = 'Invalid email address!'
        elif not re.match(r'[A-Za-z0-9]+', username):
            msg = 'Username must contain only characters and numbers!'
        elif not username or not password or not email:
            msg = 'Please fill out the form!'
        else:
            # Account doesnt exists and the form data is valid, now insert new account into accounts table
            cursor.execute('INSERT INTO accounts VALUES (NULL, %s, %s, %s)', (username, password, email,))
            mysql.connection.commit()
            msg = 'You have successfully registered!'
        cursor.close()

    elif request.method == 'POST':
        # Form is empty... (no POST data)
        msg = 'Please fill out the form!'
    # Show registration form with message (if any)
    return render_template('register.html', msg=msg)
# -------------------------- REGISTER END ------------------------------------ #

# -------------------------- HOME PAGE START --------------------------------- #

# http://localhost:5000/pythinlogin/home - this will be the home page, only accessible for loggedin users
@app.route('/pythonlogin/home')
def home():
    # Check if user is loggedin
    if 'loggedin' in session:
        # User is loggedin show them the home page
         # We need all the account info for the user so we can display it on the profile page
        cursor = mysql.connection.cursor(MySQLdb.cursors.DictCursor)
        cursor.execute('SELECT petboxs.petbox FROM accounts INNER JOIN petboxs on accounts.id = petboxs.userid WHERE accounts.id = %s', (session['id'],))
        devices = cursor.fetchone()
        cursor.close()

        return render_template('home.html', username=session['username'], devices=devices)
    # User is not loggedin redirect to login page
    return redirect(url_for('login'))

# -------------------------- HOME PAGE END -------------------------------------- #

# -------------------------- PROFILE PAGE START --------------------------------- #

# http://localhost:5000/pythinlogin/profile - this will be the profile page, only accessible for loggedin users
@app.route('/pythonlogin/profile')
def profile():
    # Check if user is loggedin
    if 'loggedin' in session:
        # We need all the account info for the user so we can display it on the profile page
        cursor = mysql.connection.cursor(MySQLdb.cursors.DictCursor)
        cursor.execute('SELECT * FROM accounts WHERE id = %s', (session['id'],))
        account = cursor.fetchone()
        cursor.close()

        # Show the profile page with account info
        return render_template('profile.html', account=account)
    # User is not loggedin redirect to login page
    return redirect(url_for('login'))

# -------------------------- PROFILE PAGE END ------------------------------------- #

# -------------------------- DASHBOARD PAGE START --------------------------------- #


@app.route('/pythonlogin/devices')
def devices():
    # Check if user is loggedin
    if 'loggedin' in session:
        # We need all the account info for the user so we can display it on the profile page
        cursor = mysql.connection.cursor(MySQLdb.cursors.DictCursor)
        cursor.execute('SELECT petboxs.petbox FROM accounts INNER JOIN petboxs on accounts.id = petboxs.userid WHERE accounts.id = %s', (session['id'],))
        devices = cursor.fetchone()
        cursor.close()

        return render_template('dashboard.html', devices=devices)
    # User is not loggedin redirect to login page
    return redirect(url_for('login'))    

# -------------------------- DASHBOARD PAGE END --------------------------------- #
# -------------------------- Register Device PAGE Start --------------------------------- #

@app.route('/addDevice', methods=['GET', 'POST'])
def addDevice():
    if request.method == 'POST' and 'deviceId' in request.form:
        # Create variables for easy access
        deviceId = request.form['deviceId']

        cursor = mysql.connection.cursor(MySQLdb.cursors.DictCursor)
        cursor.execute('SELECT * FROM petboxs WHERE petboxs.petbox = %s', (deviceId,))
        device = cursor.fetchone()
        
        # if devices are already registered, dont do anything.
        if (device == None):
            msg = 'Please check your device ID !'
        elif(device['userid'] != None):
            msg = 'Your device have been registered!'
        # if devices do not be register, update owner, and CREAT a datasheet to record data.
        elif not re.match(r'[A-Za-z0-9]+', deviceId):
            msg = 'PetboxID must contain only characters and numbers!'
        elif(device['userid'] == None):
            cursor = mysql.connection.cursor(MySQLdb.cursors.DictCursor)
            cursor.execute('UPDATE petboxs SET userid = %s WHERE petboxs.petbox = %s', (session['id'], deviceId,))
            mysql.connection.commit()
            msg = 'Sucessfully, Your device have registered!'
        cursor.close()

    elif request.method == 'POST':
        # Form is empty... (no POST data)
        msg = 'Please fill out the form!'
        # Show registration form with message (if any)
    return redirect(url_for('devices'))

# -------------------------- Register Device PAGE END --------------------------------- #

# -------------------------- RTMP STREAM PAGE START --------------------------------- #

@app.route('/video2_feed')
def video2_feed():
    return Response(gen(VideoCamera("rtmp://<rtmp_server>:1935/live/test")), mimetype='multipart/x-mixed-replace; boundary=frame')


# -------------------------- RTMP STREAM PAGE START --------------------------------- #



# -------------------------- WEBSOCKET START---------------------------------- #
# Socket Connection

    # BRIDGE WEBSOCKETIO --> MQTT BROKER

@socketio.on('subscribe')
def handle_subscribe(json):
    # print('subscribing to ' + str(json['topic']))
    mqtt.subscribe(str(json['topic']))
    
@socketio.on('publish')
def handle_publish(json):
    # print('publishing to ' + str(json['topic']), " : ", str(json['payload']))
    mqtt.publish(str(json['topic']), str(json['payload']))
    

    # BRIDGE MQTT BROKER --> WEBSOCKETIO or MySQL

mqtt.subscribe('petbox/status')

@mqtt.on_message()
def handle_mqtt_message(client, userdata, message):
    data = dict(
            topic=message.topic,
            payload=message.payload.decode()
        )

    try:
        if (data['topic'] == 'petbox/status'):
            device_data = json.loads(data['payload'])

            conn = MySQLdb.connect(host="MYSQL MASTER HOST", user = "user", passwd = "user", db = "devices")
            cursor = conn.cursor(MySQLdb.cursors.DictCursor)
            sql = 'INSERT INTO `' + device_data['deviceId'] + '` VALUES (NULL,' + str(device_data['temperature']) + ',' + str(device_data['humidity']) + ',' + device_data['waterlevel'] + ',' + device_data['feedlevel'] + ",'" + device_data['bulb'] + "','" + device_data['feed'] + "','" + device_data['FeedTime1'] + "','" + device_data['FeedTime2'] + "','" + device_data['FeedTime3'] + "','" + device_data['bulbTime1'] + "','" + device_data['bulbTime2'] + "',NULL)"
            # print(sql)
            cursor.execute(sql)
            conn.commit()
            cursor.close()
            conn.close()
        else:
            socketio.emit('mqtt', data=data)
            # print(data["topic"] + " = " + data["payload"])
    except:
        print('mqtt to mysql error')



    # GET CHART DATA 

@socketio.on('getChartData')
def handle_data_req(msg):
    conn = MySQLdb.connect(host="MYSQL_SLAVE HOST", user = "user", passwd = "user", db = "devices")
    cursor = conn.cursor(MySQLdb.cursors.DictCursor)

    sql = 'SELECT * FROM ' + str(msg['topic']) + ' order by date_time desc limit 15'
    cursor.execute(sql)
    data = cursor.fetchall()
    data = dict(
        topic = str(msg['topic']),
        payload = json.dumps(data, cls=JsonExtendEncoder)
    )
    socketio.emit('chartData_' + str(msg['topic']), data=data)
    cursor.close()
    conn.close()



# -------------------------- WEBSOCKET END ---------------------------------- #


if __name__ == "__main__":
    socketio.run(app, use_reloader=False , debug=True)


