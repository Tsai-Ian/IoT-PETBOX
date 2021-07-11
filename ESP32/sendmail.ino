#include "ESP32_MailClient.h"

#define emailSenderAccount   "Sender@example.com" // 發信者帳號
#define emailSenderPassword  "Password" // 密碼
#define emailRecipient "Receiver@example.com" // 收件者
#define smtpServer "smtp.gmail.com" //傳送email使用的server
#define smtpServerPort 465 // server的Port
#define emailSubject "testESP32t" // 定義mail的主題，方便之後修改

SMTPData smtpData; //包含要通過電子郵件發送的數據和所有其它配置

void loop_sendmail() 
{
  smtpData.setLogin(smtpServer, smtpServerPort, emailSenderAccount, emailSenderPassword); // 登入的SMTP服務器主機，SMTP端口，帳號和密碼
  smtpData.setSender("ESP32-CAM", emailSenderAccount); //設置發信者名稱和發信者帳號 
  smtpData.setPriority("High"); // 設置email優先級
  smtpData.setSubject(emailSubject); // 設置email主題
  
  // 設置消息，可以發送HTML文本或原始文本。這裡使用HTML
  smtpData.setMessage("<div style=\"color:#ff0000;font-size:20px;\">Hello World! - From ESP32</div>", true);
  // 發送原始文本
  //smtpData.setMessage("Photo captured with ESP32-CAM and attached in this email.", false);
  
  smtpData.addRecipient(emailRecipient); // 設置收件者
  
  if (!MailClient.sendMail(smtpData)) // 如果發送失敗，印出失敗+原因
  Serial.println("Error sending Email, " + MailClient.smtpErrorReason());
  
  
  smtpData.empty(); // 清除數據釋放內存
  Serial.println("send over"); // 印出已發送email
}
