setMode -pff
setMode -pff
addConfigDevice  -name "flash_100M" -path "/root/"
setSubmode -pffbpi
setAttribute -configdevice -attr multibootBpiType -value "TYPE_BPI"
setAttribute -configdevice -attr multibootBpichainType -value "PARALLEL"
addDesign -version 0 -name "0"
setMode -pff
addDeviceChain -index 0
setMode -pff
addDeviceChain -index 0
setAttribute -configdevice -attr compressed -value "FALSE"
setAttribute -configdevice -attr compressed -value "FALSE"
setAttribute -configdevice -attr autoSize -value "FALSE"
setAttribute -configdevice -attr fileFormat -value "mcs"
setAttribute -configdevice -attr fillValue -value "FF"
setAttribute -configdevice -attr swapBit -value "FALSE"
setAttribute -configdevice -attr dir -value "UP"
setAttribute -configdevice -attr multiboot -value "FALSE"
setAttribute -configdevice -attr multiboot -value "FALSE"
setAttribute -configdevice -attr spiSelected -value "FALSE"
setAttribute -configdevice -attr spiSelected -value "FALSE"
setAttribute -configdevice -attr ironhorsename -value "1"
setAttribute -configdevice -attr flashDataWidth -value "16"
setCurrentDesign -version 0
setAttribute -design -attr RSPin -value ""
setCurrentDesign -version 0
addPromDevice -p 1 -size 131072 -name 128M
setMode -pff
setMode -pff
setMode -pff
setMode -pff
addDeviceChain -index 0
setMode -pff
addDeviceChain -index 0
setMode -pff
setSubmode -pffbpi
setMode -pff
setAttribute -design -attr RSPin -value "00"
addDevice -p 1 -file "/root/Documents/spaiagua/Swinger/pcie_bridge_simple/implementation/system.bit"
setAttribute -design -attr RSPinMsb -value "1"
setAttribute -design -attr name -value "0"
setAttribute -design -attr RSPin -value "00"
setAttribute -design -attr endAddress -value "ce77e3"
setAttribute -design -attr endAddress -value "ce77e3"
setMode -pff
setSubmode -pffbpi
generate
setCurrentDesign -version 0
setMode -bs
setMode -bs
setMode -bs
setMode -bs
setCable -port auto
Identify -inferir 
identifyMPM 
attachflash -position 1 -bpi "28F00AG18F"
assignfiletoattachedflash -position 1 -file "/root/flash_100M.mcs"
Program -p 1 -dataWidth 16 -rs1 25 -rs0 24 -bpionly -e -v -loadfpga 
setMode -bs
setMode -bs
setMode -ss
setMode -sm
setMode -hw140
setMode -spi
setMode -acecf
setMode -acempm
setMode -pff
setMode -bs
saveProjectFile -file "/root//auto_project.ipf"
setMode -bs
setMode -pff
setMode -bs
deleteDevice -position 1
setMode -bs
setMode -ss
setMode -sm
setMode -hw140
setMode -spi
setMode -acecf
setMode -acempm
setMode -pff
