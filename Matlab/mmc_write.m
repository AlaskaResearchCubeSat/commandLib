function mmc_write(sobj,sector,data)
%mmc_write write data to mmc card on MSP
%   sobj - serial object to communicate with
%   data - data to write. data will be typecast to uint8 and padded to 512 
%          bytes before sending so make sure that it is in the propper form

    %typecast and reshape
    data=typecast(reshape(data,1,[]),'uint8');
    %pad data
    data=[data,zeros(1,512*ceil(length(data)/512)-length(data))];
    %send command
    command(sobj,'mmcdread %lu %u',sector,length(data)/512);
    %get line with error or sector confermation
    line=fgetl(sobj);
    %check for error
    if(strncmpi('Error',line,length('Error')))    
        %wait for command to finish
        waitReady(sobj);
        error('Failed to write data: ''%s''',deblank(line))
    end
    %unknown command message
    umsg='unknown command ''mmcdread''';
    %check for unknown command
    if(strncmpi(umsg,line,length(umsg)))
        %wait for command to finish
        waitReady(sobj);
        error('mmcdread command not avaliable on microcontroller');
    end
    s=sscanf(line,'Starting at MMC block %lu');
    %check to see if data was parsed
    if ~all(size(s)==[1,1])
        %wait for command to finish
        waitReady(sobj);
        error('Error reading block number. failed to parse "%s"',deblank(line));
    end
    %check that sectors match
    if s~=sector
        waitReady(sobj);
        error('Error sector mismatch. Expected %lu got %lu',sector,s)
    end

    for k=32:32:length(data)
        %write a bit of data
        fwrite(sobj,data((k-31):k),'uint8');
        %get check line
        line=fgetl(sobj);
        %parse check
        check=sscanf(line,'%u');
        %chekc that data was parsed
        if ~all(size(check)==[1 1])
            %wait for command to finish
            waitReady(sobj);
            error('Error reading check. failed to parse "%s"',deblank(line));
        end
        %calculate check
        calc=mod(sum(data((k-31):k)),2^16);
        %check that check and calculated check match
        if check~=calc
            %wait for command to finish
            waitReady(sobj);
            error('Error Check failed. Check = %u Calculated = %u',check,calc);
        end
    end

end

