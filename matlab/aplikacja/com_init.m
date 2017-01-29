global s
s=serial('COM12');
set(s,'BaudRate',921600,'DataBits',8,'Parity','none','StopBits',1,'FlowControl','none','Timeout',100);
fopen(s);