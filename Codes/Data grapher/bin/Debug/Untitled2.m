clear
clc
address='exp10X\exp101\exp101_fingers.csv';
T = readtable(address);
Data = table2array(T);
Datalpf=Data;
[m,n] = size(Data);
mm=m/3;
for j=1:n
for i=1:mm
    Data2(i,j)=(Data(3*i,j)+Data(3*i-1,j)+Data(3*i-2,j))/3;
end
end

for i=1:n
    Data3(:,i)=movmean(Data2(:,i),100);
end

csvwrite(strcat('fingers.csv'),Data3);