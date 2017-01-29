function [out, data_size] = com_receive()
global s;

    N = s.BytesAvailable;
    
    if (N<7)
        out = 0; data_size = 0;
        return;
    end    
    [msg, N, status]=fread(s, N);
    %status;
    %msg;
    
    data_size = N - 6;
    data_in = uint8(zeros(1, data_size));
    for i=1:data_size
        data_in(i) = uint8(msg(i+4));
    end
    
    out = data_in;
end