global s
s=serial('COM27');
set(s,'BaudRate',115200,'DataBits',8,'Parity','none','StopBits',1,'FlowControl','none','Timeout',100);
fopen(s);