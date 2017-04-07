/*Print IP of local host, interal-net Subnet Mask,IP of GateWay And MAC address of local host*/
#include <windows.h>
#include <stdio.h>
#include <Iphlpapi.h>

#pragma comment(lib,"Iphlpapi.lib")
#pragma comment(lib,"WS2_32.lib")

unsigned char local_mac[6]; //mac address of local host
DWORD         gateway_ip;     // IP of gateway
DWORD         local_ip;       //IP of local host
DWORD         sub_net_mask;   // sub Net Mask

bool get_global_data(){
	
	PIP_ADAPTER_INFO pAdapter_info = NULL;
	unsigned long len = 0;
	
	//alloc memory for adapter struct
	GetAdaptersInfo(pAdapter_info,&len);
	pAdapter_info = (PIP_ADAPTER_INFO)GlobalAlloc(GPTR,len);

	//get adapter infor of local host
	if(GetAdaptersInfo(pAdapter_info,&len) == ERROR_SUCCESS){
		if(pAdapter_info != NULL){
			
			memcpy(local_mac,pAdapter_info->Address,6);
			
			gateway_ip = (DWORD)inet_addr(pAdapter_info->GatewayList.IpAddress.String);     
			local_ip = (DWORD)inet_addr(pAdapter_info->IpAddressList.IpAddress.String);       
			sub_net_mask = (DWORD)inet_addr(pAdapter_info->IpAddressList.IpMask.String); 
		}
	}

	printf("***********************Local Host Info****************\n");
	in_addr in_dis;

	in_dis.S_un.S_addr = local_ip;
	printf("ip address is %s\n",inet_ntoa(in_dis));

	in_dis.S_un.S_addr = sub_net_mask;
	printf("Subnet Mask is %s\n",inet_ntoa(in_dis));

	in_dis.S_un.S_addr = gateway_ip;
	printf("default gateway is %s\n",inet_ntoa(in_dis));

	unsigned char* p = local_mac;

	printf("Mac Address is:%02x-%02x-%02x-%02x-%02x-%02x\n",p[0],p[1],p[2],p[3],p[4],p[5]);


	return true;
}
int main(){
	
	get_global_data();
	return 0;
}