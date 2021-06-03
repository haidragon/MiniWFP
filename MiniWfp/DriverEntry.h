#pragma once
////////////////////////////////////
//			  NT �ļ�			  //	
////////////////////////////////////
#include <ntddk.h>

////////////////////////////////////
//			  WPF �ļ�			  //	
////////////////////////////////////
#include <fwpmk.h>
#include <fwpsk.h>
#define INITGUID
#include <guiddef.h>

extern "C"
{
	////////////////////////////////////
	//			  ��������			  //	
	////////////////////////////////////
	// {D969FC67-6FB2-4504-91CE-A97C3C32AD36}
	DEFINE_GUID(WFP_SAMPLE_ESTABLISHED_CALLOUT_V4_GUID, 0xd969fc67, 0x6fb2, 0x4504, 0x91, 0xce, 0xa9, 0x7c, 0x3c, 0x32, 0xad, 0x36);

	// {ED6A516A-36D1-4881-BCF0-ACEB4C04C21C}
	DEFINE_GUID(WFP_SAMPLE_SUBLAYER_GUID,
		0xed6a516a, 0x36d1, 0x4881, 0xbc, 0xf0, 0xac, 0xeb, 0x4c, 0x4, 0xc2, 0x1c);

	////////////////////////////////////
	//			  ��������			  //	
	////////////////////////////////////
	/* �����豸���� */
	PDEVICE_OBJECT	CreateDevice(__in struct _DRIVER_OBJECT* DriverObject);
	/* ��ʼ��WFP */
	NTSTATUS InitWfp();
	/* ��ȡ�������� */
	HANDLE OpenEngine();
	/* ע������ӿ� */
	NTSTATUS WfpRegisterCallouts(IN OUT void* deviceObject);
	/* ʵ�ֽӿں�����ע�� */
	NTSTATUS WfpRegisterCalloutImple(
		IN OUT void* deviceObject,
		IN  FWPS_CALLOUT_CLASSIFY_FN ClassifyFunction,
		IN  FWPS_CALLOUT_NOTIFY_FN NotifyFunction,
		IN  FWPS_CALLOUT_FLOW_DELETE_NOTIFY_FN FlowDeleteFunction,
		IN  GUID const* calloutKey,
		IN  UINT32 flags,
		OUT UINT32* calloutId
	);
	/* �����������ӽӿں��� */
	NTSTATUS WfpAddCallouts();
	/* ����Ӳ� */
	NTSTATUS WfpAddSubLayer();
	/* ��ӹ����� */
	NTSTATUS WfpAddFilters();
	/* ж��WFP */
	VOID UninitWfp();
	/* ɾ���豸���� */
	VOID DeleteDevice();

	void Wfp_Sample_Established_ClassifyFn_V4(
		_In_ const FWPS_INCOMING_VALUES* inFixedValues,
		_In_ const FWPS_INCOMING_METADATA_VALUES* inMetaValues,
		_Inout_opt_ void* layerData,
		_In_opt_ const void* classifyContext,
		_In_ const FWPS_FILTER* filter,
		_In_ UINT64 flowContext,
		_Inout_ FWPS_CLASSIFY_OUT* classifyOut
	);

	NTSTATUS NTAPI Wfp_Sample_Established_NotifyFn_V4(
		_In_ FWPS_CALLOUT_NOTIFY_TYPE notifyType,
		_In_ const GUID* filterKey,
		_Inout_ FWPS_FILTER* filter
	);

	void NTAPI Wfp_Sample_Established_FlowDeleteFn_V4(
		_In_ UINT16 layerId,
		_In_ UINT32 calloutId,
		_In_ UINT64 flowContext
	);
}
