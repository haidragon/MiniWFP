#include "DriverEntry.h"

#define HTTP_DEFAULT_PORT 80

extern "C"
{
	PCHAR PsGetProcessImageFileName(PEPROCESS Process); // ��ȡ�ں˶���Ľ�������

	CHAR* DirectionName = NULL; CHAR* ProtocalName = NULL;
	void Wfp_Sample_Established_ClassifyFn_V4(
		_In_ const FWPS_INCOMING_VALUES* inFixedValues,
		_In_ const FWPS_INCOMING_METADATA_VALUES* inMetaValues,
		_Inout_opt_ void* layerData,
		_In_opt_ const void* classifyContext,
		_In_ const FWPS_FILTER* filter,
		_In_ UINT64 flowContext,
		_Inout_ FWPS_CLASSIFY_OUT* classifyOut
	)
	{
		WORD	wDirection = 0;
		WORD	wRemotePort = 0;
		WORD	wSrcPort = 0;
		WORD	wProtocol = 0;
		ULONG	ulSrcIPAddress = 0;
		ULONG	ulRemoteIPAddress = 0;

		if (!(classifyOut->rights & FWPS_RIGHT_ACTION_WRITE))
		{
			return;
		}
		//wDirection��ʾ���ݰ��ķ���,ȡֵΪ	//FWP_DIRECTION_INBOUND/FWP_DIRECTION_OUTBOUND
		wDirection = inFixedValues->incomingValue[FWPS_FIELD_ALE_FLOW_ESTABLISHED_V4_DIRECTION].value.int8;

		//wSrcPort��ʾ���ض˿ڣ�������
		wSrcPort = inFixedValues->incomingValue[FWPS_FIELD_ALE_FLOW_ESTABLISHED_V4_IP_LOCAL_PORT].value.uint16;

		//wRemotePort��ʾԶ�˶˿ڣ�������
		wRemotePort = inFixedValues->incomingValue[FWPS_FIELD_ALE_FLOW_ESTABLISHED_V4_IP_REMOTE_PORT].value.uint16;

		//ulSrcIPAddress ��ʾԴIP
		ulSrcIPAddress = inFixedValues->incomingValue[FWPS_FIELD_ALE_FLOW_ESTABLISHED_V4_IP_LOCAL_ADDRESS].value.uint32;

		//ulRemoteIPAddress ��ʾԶ��IP
		ulRemoteIPAddress = inFixedValues->incomingValue[FWPS_FIELD_ALE_FLOW_ESTABLISHED_V4_IP_REMOTE_ADDRESS].value.uint32;

		//wProtocol��ʾ����Э�飬����ȡֵ��IPPROTO_ICMP/IPPROTO_UDP/IPPROTO_TCP
		wProtocol = inFixedValues->incomingValue[FWPS_FIELD_ALE_FLOW_ESTABLISHED_V4_IP_PROTOCOL].value.uint8;

		//Ĭ��"����"(PERMIT)
		classifyOut->actionType = FWP_ACTION_PERMIT;
		
		//�򵥵Ĳ����жϣ�������д�ⲿ��
		if ((wProtocol == IPPROTO_TCP) &&
			(wDirection == FWP_DIRECTION_OUTBOUND) &&
			(wRemotePort == HTTP_DEFAULT_PORT))
		{
			//TCPЭ�鳢�Է���80�˿ڵķ��ʣ�����(BLOCK)
			classifyOut->actionType = FWP_ACTION_BLOCK;

			/*
				��д YOUS CODE...
			*/
		
			switch (wDirection)
			{
			case FWP_DIRECTION_INBOUND:
				DirectionName = "Recv"; break;
			case FWP_DIRECTION_OUTBOUND:
				DirectionName = "Send"; break;
			}
		
			switch (wProtocol)
			{
			case IPPROTO_ICMP:
				ProtocalName = "ICMP"; break;
			case IPPROTO_UDP:
				ProtocalName = "UDP";  break;
			case IPPROTO_TCP:
				ProtocalName = "TCP";  break;
			}

			DbgPrint("[%s]--%s:[%s], LocalIp=[%u.%u.%u.%u:%d], RemoteIp=[%u.%u.%u.%u:%d], PID=[%I64d]\r\n",
				PsGetProcessImageFileName(IoGetCurrentProcess()),
				DirectionName,
				ProtocalName,
				(ulSrcIPAddress >> 24) & 0xFF,
				(ulSrcIPAddress >> 16) & 0xFF,
				(ulSrcIPAddress >> 8) & 0xFF,
				(ulSrcIPAddress) & 0xFF,
				wSrcPort,
				(ulRemoteIPAddress >> 24) & 0xFF,
				(ulRemoteIPAddress >> 16) & 0xFF,
				(ulRemoteIPAddress >> 8) & 0xFF,
				(ulRemoteIPAddress) & 0xFF,
				wRemotePort,
				inMetaValues->processId
			);
		}

		//���FWPS_RIGHT_ACTION_WRITE���
		if (filter->flags & FWPS_FILTER_FLAG_CLEAR_ACTION_RIGHT)
		{
			classifyOut->rights &= ~FWPS_RIGHT_ACTION_WRITE;
		}
		return;
	}

	NTSTATUS NTAPI Wfp_Sample_Established_NotifyFn_V4(
		_In_ FWPS_CALLOUT_NOTIFY_TYPE notifyType,
		_In_ const GUID* filterKey,
		_Inout_ FWPS_FILTER* filter
	)
	{
		return STATUS_SUCCESS;
	}

	void NTAPI Wfp_Sample_Established_FlowDeleteFn_V4(
		_In_ UINT16 layerId,
		_In_ UINT32 calloutId,
		_In_ UINT64 flowContext)
	{
		return;
	}
}


