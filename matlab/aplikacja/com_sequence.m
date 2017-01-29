N = 500;
dataTab = zeros(1, N);
flushinput(s);
tic;
for i = 1:N
    com_send([0]);
    tau = toc;
    wait(tau, 0.04);
    [data, data_size] = com_receive();
    
    if (data_size>0)
        force = typecast(data, 'int16');
        dataTab(i) = force;
    end
end    