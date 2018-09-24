# This will have a SwitchType property. As far as I know the values are:
#
#   0 - Private
#   1 - Internal
#

$Switches = @(Hyper-V\Get-VMSwitch | Select-Object Name,SwitchType,NetAdapterInterfaceDescription,Id)
Write-Host $(ConvertTo-JSON $Switches)
