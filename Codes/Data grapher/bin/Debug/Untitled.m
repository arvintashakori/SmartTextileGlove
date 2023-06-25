clear
clc
t=1:3000;
y=3000-t+100*rand(1,3000);

for i=1:2998
    z(i)=y(i+2)-2*y(i+1)+y(i);
end