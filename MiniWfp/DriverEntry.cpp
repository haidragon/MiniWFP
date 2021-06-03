#include "DriverEntry.h"

#define WFP_DEVICE_NAME				L"\\Device\\wfp_device"

#define WFP_SYM_LINK_NAME			L"\\??\\wfp_device"

#define WFP_SAMPLE_ESTABLISHED_CALLOUT_DISPLAY_NAME	L"WfpSampleEstablishedCalloutName"

#define WFP_SAMPLE_SUB_LAYER_DISPLAY_NAME	L"WfpSampleSubLayerName"

#define WFP_SAMPLE_FILTER_ESTABLISH_DISPLAY_NAME	L"WfpSampleFilterEstablishName"

extern "C" 
{
	DRIVER_INITIALIZE	DriverEntry;
	DRIVER_UNLOAD		DrUnload;

	PDEVICE_OBJECT	g_pDeviceObj;
	HANDLE			g_hEngine;
	UINT32	g_uFwpsEstablishedCallOutId = 0;
	UINT32	g_uFwpmEstablishedCallOutId = 0;
	UINT64	g_uEstablishedFilterId = 0;

	NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath)
	{
		UNREFERENCED_PARAMETER(DriverObject);
		UNREFERENCED_PARAMETER(RegistryPath);
		NTSTATUS NtStatus = STATUS_UNSUCCESSFUL;
		DriverObject->DriverUnload = DrUnload;
		
		do
		{
			/* �����豸���� */
			g_pDeviceObj = CreateDevice(DriverObject);
			if (g_pDeviceObj == NULL)
			{
				break;
			}
			/* ��ʼ��WFP */
			if (InitWfp() != STATUS_SUCCESS)
			{
				break;
			}

			NtStatus = STATUS_SUCCESS;
			KdPrint(("��������! \r\n"));
		} while (FALSE);
		return NtStatus;
	}

	VOID DrUnload(_In_ PDRIVER_OBJECT DriverObject)
	{
		/* ж��WFP */
		UninitWfp();
		/* ɾ���豸���� */
		DeleteDevice();
		KdPrint(("����ж��! \r\n"));
	}

	PDEVICE_OBJECT	CreateDevice(__in struct _DRIVER_OBJECT* DriverObject)
	{
		UNICODE_STRING	uDeviceName = { 0 };
		UNICODE_STRING	uSymbolName = { 0 };
		PDEVICE_OBJECT	pDeviceObj = NULL;
		NTSTATUS nStatsus = STATUS_UNSUCCESSFUL;
		RtlInitUnicodeString(&uDeviceName, WFP_DEVICE_NAME);
		RtlInitUnicodeString(&uSymbolName, WFP_SYM_LINK_NAME);
		nStatsus = IoCreateDevice(DriverObject, 0, &uDeviceName, FILE_DEVICE_UNKNOWN, 0, FALSE, &pDeviceObj);
		if (pDeviceObj != NULL)
		{
			pDeviceObj->Flags |= DO_BUFFERED_IO;
		}
		IoCreateSymbolicLink(&uSymbolName, &uDeviceName);
		return pDeviceObj;
	}

	NTSTATUS InitWfp()
	{
		NTSTATUS NtStatus = STATUS_UNSUCCESSFUL;

		do
		{
			/* ��ȡ�������� */
			g_hEngine = OpenEngine();
			if (g_hEngine == NULL)
			{
				break;
			}
			/* ע������ӿں��� */
			if (STATUS_SUCCESS != WfpRegisterCallouts(g_pDeviceObj))
			{
				break;
			}
			/* �����������ӽӿں��� */
			if (STATUS_SUCCESS != WfpAddCallouts())
			{
				break;
			}
			/* ����Ӳ� */
			if (STATUS_SUCCESS != WfpAddSubLayer())
			{
				break;
			}
			/* ��ӹ����� */
			if (STATUS_SUCCESS != WfpAddFilters())
			{
				break;
			}

			NtStatus = STATUS_SUCCESS;
		} while (FALSE);

		return NtStatus;
	}

	HANDLE OpenEngine()
	{
		FWPM_SESSION0 Session = { 0 };
		HANDLE hEngine = NULL;
		FwpmEngineOpen(NULL, RPC_C_AUTHN_WINNT, NULL, &Session, &hEngine);
		return hEngine;
	}

	NTSTATUS WfpRegisterCallouts(IN OUT void* deviceObject)
	{
		NTSTATUS NtStatus = STATUS_UNSUCCESSFUL;

		do
		{
			/* �����ӿں��� */
			NtStatus = WfpRegisterCalloutImple(deviceObject,
				Wfp_Sample_Established_ClassifyFn_V4,
				Wfp_Sample_Established_NotifyFn_V4,
				Wfp_Sample_Established_FlowDeleteFn_V4,
				&WFP_SAMPLE_ESTABLISHED_CALLOUT_V4_GUID,
				0,
				&g_uFwpsEstablishedCallOutId);
			if (NtStatus != STATUS_SUCCESS)
			{
				break;
			}
			NtStatus = STATUS_SUCCESS;
		} while (FALSE);
		return NtStatus;
	}

	NTSTATUS WfpRegisterCalloutImple(
		IN OUT void* deviceObject,
		IN  FWPS_CALLOUT_CLASSIFY_FN ClassifyFunction,
		IN  FWPS_CALLOUT_NOTIFY_FN NotifyFunction,
		IN  FWPS_CALLOUT_FLOW_DELETE_NOTIFY_FN FlowDeleteFunction,
		IN  GUID const* calloutKey,
		IN  UINT32 flags,
		OUT UINT32* calloutId
	)
	{
		FWPS_CALLOUT sCallout;
		NTSTATUS status = STATUS_SUCCESS;

		memset(&sCallout, 0, sizeof(FWPS_CALLOUT));

		sCallout.calloutKey = *calloutKey;
		sCallout.flags = flags;
		sCallout.classifyFn = ClassifyFunction;
		sCallout.notifyFn = NotifyFunction;
		sCallout.flowDeleteFn = FlowDeleteFunction;

		status = FwpsCalloutRegister(deviceObject, &sCallout, calloutId);
		return status;
	}

	NTSTATUS WfpAddCallouts()
	{
		NTSTATUS NtStatus = STATUS_SUCCESS;
		FWPM_CALLOUT fwpmCallout = { 0 };
		fwpmCallout.flags = 0;
		do
		{
			if (g_hEngine == NULL)
			{
				break;
			}
			fwpmCallout.displayData.name = (wchar_t *)WFP_SAMPLE_ESTABLISHED_CALLOUT_DISPLAY_NAME;
			fwpmCallout.displayData.description = (wchar_t *)WFP_SAMPLE_ESTABLISHED_CALLOUT_DISPLAY_NAME;
			fwpmCallout.calloutKey = WFP_SAMPLE_ESTABLISHED_CALLOUT_V4_GUID;
			fwpmCallout.applicableLayer = FWPM_LAYER_ALE_FLOW_ESTABLISHED_V4;
			NtStatus = FwpmCalloutAdd(g_hEngine, &fwpmCallout, NULL, &g_uFwpmEstablishedCallOutId);
			if (!NT_SUCCESS(NtStatus) && (NtStatus != STATUS_FWP_ALREADY_EXISTS))
			{
				break;
			}
			NtStatus = STATUS_SUCCESS;
		} while (FALSE);
		return NtStatus;
	}

	NTSTATUS WfpAddSubLayer()
	{
		NTSTATUS NtStatus = STATUS_UNSUCCESSFUL;
		FWPM_SUBLAYER SubLayer = { 0 };
		SubLayer.flags = 0;
		SubLayer.displayData.description = WFP_SAMPLE_SUB_LAYER_DISPLAY_NAME;
		SubLayer.displayData.name = WFP_SAMPLE_SUB_LAYER_DISPLAY_NAME;
		SubLayer.subLayerKey = WFP_SAMPLE_SUBLAYER_GUID;
		SubLayer.weight = 65535; /* Ȩ�� */
		if (g_hEngine != NULL)
		{
			NtStatus = FwpmSubLayerAdd(g_hEngine, &SubLayer, NULL);
		}
		return NtStatus;
	}

	NTSTATUS WfpAddFilters()
	{
		NTSTATUS NtStatus = STATUS_UNSUCCESSFUL;
		do
		{
			FWPM_FILTER0 Filter = { 0 };
			FWPM_FILTER_CONDITION FilterCondition[1] = { 0 };
			FWP_V4_ADDR_AND_MASK AddrAndMask = { 0 };
			if (g_hEngine == NULL)
			{
				break;
			}
			Filter.displayData.description = WFP_SAMPLE_FILTER_ESTABLISH_DISPLAY_NAME;
			Filter.displayData.name = WFP_SAMPLE_FILTER_ESTABLISH_DISPLAY_NAME;
			Filter.flags = 0;
			Filter.layerKey = FWPM_LAYER_ALE_FLOW_ESTABLISHED_V4;	/* ָ���ֲ��GUID */
			Filter.subLayerKey = WFP_SAMPLE_SUBLAYER_GUID;			/* ָ���Ӳ��GUID */
			Filter.weight.type = FWP_EMPTY;							/* �Զ�����Ȩ�� */
			Filter.numFilterConditions = 1;
			Filter.filterCondition = FilterCondition;				/* �����������(Ψһ) */
			Filter.action.type = FWP_ACTION_CALLOUT_TERMINATING;	/* �ͺ����ӿ���������Ҹýӿ�ֻ�ܷ���"����"��"����" */
			Filter.action.calloutKey = WFP_SAMPLE_ESTABLISHED_CALLOUT_V4_GUID; /* ָ�������ӿڵ�GUID */

			FilterCondition[0].fieldKey = FWPM_CONDITION_IP_REMOTE_ADDRESS;	/* ��������ֶ�ΪԶ��IP */
			FilterCondition[0].matchType = FWP_MATCH_EQUAL;					/* ƥ���������ݰ� */
			FilterCondition[0].conditionValue.type = FWP_V4_ADDR_MASK;
			FilterCondition[0].conditionValue.v4AddrMask = &AddrAndMask;
			NtStatus = FwpmFilterAdd(g_hEngine, &Filter, NULL, &g_uEstablishedFilterId);
			if (STATUS_SUCCESS != NtStatus)
			{
				break;
			}
			NtStatus = STATUS_SUCCESS;
		} while (FALSE);
		return NtStatus;
	}

	VOID UninitWfp()
	{
		if (g_hEngine != NULL)
		{
			/* �Ƴ������� */
			FwpmFilterDeleteById(g_hEngine, g_uEstablishedFilterId);
		}
		if (g_hEngine != NULL)
		{
			/* �Ƴ��Ӳ� */
			FwpmSubLayerDeleteByKey(g_hEngine, &WFP_SAMPLE_SUBLAYER_GUID);
		}
		if (g_hEngine != NULL)
		{
			/* �ӹ��������Ƴ��ӿں��� */
			FwpmCalloutDeleteById(g_hEngine, g_uFwpmEstablishedCallOutId);
			g_uFwpmEstablishedCallOutId = 0;
		}
		if (g_hEngine != NULL) {
			/* ���Ƴ��ӿں��� */
			FwpsCalloutUnregisterById(g_uFwpsEstablishedCallOutId);
			g_uFwpsEstablishedCallOutId = 0;
		}
		if (g_hEngine != NULL)
		{
			/* ����������� */
			FwpmEngineClose(g_hEngine);
			g_hEngine = NULL;
		}
	}

	VOID DeleteDevice()
	{
		UNICODE_STRING uSymbolName = { 0 };
		RtlInitUnicodeString(&uSymbolName, WFP_SYM_LINK_NAME);
		IoDeleteSymbolicLink(&uSymbolName);
		if (g_pDeviceObj != NULL)
		{
			IoDeleteDevice(g_pDeviceObj);
		}
		g_pDeviceObj = NULL;
	}
}


