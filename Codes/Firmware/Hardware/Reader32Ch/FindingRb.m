Gain = 1:15;
Resistor = 100:100:10000;

VB = 1.2;

LowBound1 = 40;
LowBound2 = 50;

HighBound1 = 1490;
HighBound2 = 1500;

VOutHigh1 = zeros();
VOutHigh2 = zeros();
VOut = zeros();
Error = zeros();
Index = zeros();
FinalGain = zeros();
FinalResistor = zeros();
i = 1;

for n = 1:15
    for m = 1 : 100
      VLow1 =  VB*LowBound1/(LowBound1+Resistor(m));
      VLow2 = VB*LowBound2/(LowBound2+Resistor(m));
      VHigh1 = VB*HighBound1/(HighBound1+Resistor(m));
      VHigh2 = VB*HighBound2/(HighBound2+Resistor(m));
      
      DeltaLow = VLow2 - VLow1;
      DeltaHigh = VHigh2 - VHigh1;
      
      AmpLow1 = Gain(n) * VLow1;
      AmpLow2 = Gain(n) * VLow2;
      
      VOutHigh1(n*m) = Gain(n) * VHigh1;
      VOutHigh2(n*m) = Gain(n) * VHigh2;
      
      if (VOutHigh1(n*m) < 2.5) && (VOutHigh2(n*m) < 2.5)        
        AmpDiffLow = Gain(n) * DeltaLow;
        AmpDiffHigh = Gain (n) * DeltaHigh;
        er = 20 * 3.3 /4096 * 10 / AmpDiffHigh / 2;
        if (er < 8)
            Error(i) = er;
            VOut(i) = VOutHigh2(n*m);
            Index(i) = i;
            FinalGain(i) = Gain(n);
            FinalResistor(i) = Resistor(m);
            
            i = i + 1;
        end
      end
    end
end

MinError = min(Error);
disp(MinError);

for idx = 1:numel(Error)
    if Error(idx)==MinError
        disp(VOut(idx));
        disp(FinalGain(idx));
        disp(FinalResistor(idx));
    end
end



x = 1:1:(i-1);
figure
ax1 = subplot (2,1,1);
scatter(ax1,x , VOut)
title(ax1, "VOutHigh2")
ylabel(ax1, "V")

ax2 = subplot (2,1,2);
scatter(ax2, x, Error)
title(ax2, "Error")
ylabel (ax2, "Ohm")
      
      
