directoryFrom = './2010_10_13/';
directoryTo = './2010_10_13/renamed/';
stringBase = 'calib';
stringExt = '.tif';
baseLength = length(stringBase);
extLength = length(stringExt);

for camNum=1:3
    files = dir ([directoryFrom stringBase num2str(camNum) '*' stringExt]);
    %c++ code requires one image just to pull image info from
    currName = files(1).name;
    %label one image (doesn't matter what) as image 0
    indDash = findstr(currName, '-');
    reName = [directoryTo currName(1:indDash) num2str(0) stringExt];
    copyfile([directoryFrom currName], reName);

    renameNum = 1;
    %first, get ims with 1 di
    for digitNums = 1:4
        for file = 1:length(files)
            currName = files(file).name;
            if (length(currName) == baseLength+2+extLength+digitNums)
                indDash = findstr(currName, '-');
                reName = [directoryTo currName(1:indDash) num2str(renameNum) stringExt];
                %currName
                %reName
                [directoryFrom currName]
                reName
                copyfile([directoryFrom currName], reName);                
                renameNum = renameNum+1;
            end
        end
    end
    
end