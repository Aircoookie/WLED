#pragma once

#include <BLEDevice.h>

void unPairAllDevices() {

/*********************************************************** 
	* Get the numbers of bonded/paired devices to the BT module 
	*  to update the WT12 emulation
	************************************************************/
	int wPairedDevices = esp_ble_get_bond_device_num();
	if (wPairedDevices)
	{
    esp_ble_bond_dev_t* pairedDevices = (esp_ble_bond_dev_t*)malloc(wPairedDevices * sizeof(esp_ble_bond_dev_t));
    esp_ble_get_bond_device_list(&wPairedDevices, pairedDevices);
		for (int wDeviceToGet = 0; wDeviceToGet < wPairedDevices; wDeviceToGet++)
		{
			 esp_ble_remove_bond_device(pairedDevices[wDeviceToGet].bd_addr);
 		}	
	}
}  