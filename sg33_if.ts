namespace sg33
{
	begin()
	
	//%shim=sg33::begin
	export function begin():boolean
	{
		return true
	}
	
	//%shim=sg33::IAQmeasure
	export function IAQmeasure():boolean
	{
		return true
	}
	
	//%shim=sg33::getTVOC
	export function getTVOC():number
	{
		return 1
	}
	
	//%shim=sg33::getCO2
	export function getCO2():number
	{
		return 1
	}
	
	//%block="SG33 get TVOC"
	export function TVOC():number
	{
		IAQmeasure()
		return getTVOC()
	}
	
	//%block="SG33 get eCO2"
	export function eCO2():number
	{
		IAQmeasure()
		return getCO2()
	}
}