#!/bin/bash
#To execute it do: (nohup ./LoopOn_Tolerance_Weight.sh > log.loop) >& error.file

cat > TOLERANCE << EOF
10.
1.
0.1
0.01
EOF

cat > WEIGHT << EOF
1.
1.e-1
1.e-2
1.e-3
1.e-4
1.e-5
1.e-6
1.e-7
EOF

rm -f log.tolweight

for TOLE in `cat TOLERANCE`
do
  
  for WGT in `cat WEIGHT`
  do
     echo "Iteration Tolerance=$TOLE Weight=$WGT" >>  log.tolweight
     sed "s/TOLERANCE/$TOLE/;s/WEIGHT/$WGT/" MyConfig_Red.skel > Config.TRACS_"$TOLE"_"$WGT"
     #ln -sf Config.TRACS_"$TOLE"_"$WGT" Config.TRACS
     ../MfgTRACSFit 1 2015-09-01_15-21-40_PIN_7859_1.8_4_W2_J6-1_Vbias100-500.txt.root Config.TRACS_"$TOLE"_"$WGT" >> log_"$TOLE"_"$WGT"
     mv output.root output_"$TOLE"_"$WGT".root
     echo "Done with Iteration Tolerance=$TOLE Weight=$WGT" >>  log.tolweight
  done
  
  
done

#1.e-1
