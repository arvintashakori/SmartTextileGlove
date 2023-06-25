clear
clc
%2 HOURS TEST
%address='longtest5.csv';
%7 HOURS TEST
address='longtest8.csv';

T = readtable(address);
Data = table2array(T);
Datalpf=Data;
[m,n] = size(Data);
for i=1:n
    Datalpf(:,i)=movmean(Data(:,i),1000);
end
for i=1:n
    Datalpf1(:,i)=detrend(Datalpf(:,i),'linear');
end

for i=1:n
baseline(:,i)=Datalpf(:,i)-Datalpf1(:,i);
Datalpf2(:,i)=Datalpf1(:,i)+baseline(1,i);
end

%polyfit
x=1:m;
x=x';
for i=1:n
    p(i,:)=polyfit(x,Datalpf(:,i),3);
   
    y(:,i)=polyval(p(i,:),x);
    for j=2:m
        yy(j,i)=y(j,i)-y(j-1,i);
    end
end

%for i=1:n
i=2
close all;
plot(Datalpf(:,i));
hold on
xx=ones(m,1)*polyval(p(i,:),1);
%plot(polyval(p(i,:),x));
%f = fit(x,y,'exp1')
%plot(x,xx);
f = fit(x,Datalpf(:,i),'exp1')
%plot(polyval(p(i,:),1)+Datalpf(:,i)-polyval(p(i,:),x));
plot(f,x,Datalpf(:,i))
p(i,:)
%csvwrite(strcat('sensor_baseline_slope.csv'),yy);
%csvwrite(strcat('sensor_baseline_amplitute.csv'),y);
%pause(1)
%end