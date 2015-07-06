using UnityEngine;
using System;
using System.Collections;
using System.Runtime.InteropServices;

[AttributeUsage (AttributeTargets.Method)]
public sealed class MonoPInvokeCallbackAttribute : Attribute
{
	private Type type;
	public MonoPInvokeCallbackAttribute( Type t ) { type = t; }
}

public enum VMOTION_RESULT
{
	VMOTION_DISCONNECTED = -4,
	VMOTION_OUT_OF_RANGE = -3,
	VMOTION_INVALID_ARGUMENT = -2,
	VMOTION_ERROR = -1,
	VMOTION_INSUCCESS = 0,
	VMOTION_SUCCESS = 1
}

public class VMotionInternals
{
	public delegate void VMotionConnectionDelegate(bool success);
	public static event VMotionConnectionDelegate OnVMotionConnection;

	public static uint timeout = 0;
	
	private VMotionInternals() {}

	#region dll imports
	
	private delegate void VMotionControllerConnectionCB(IntPtr sender, int connected);

	[DllImport ("VMotionLib")]
	private static extern int VMotionInit(VMotionControllerConnectionCB cb, string device_path);

	[DllImport ("VMotionLib")]
	private static extern int VMotionShutdown();

	[DllImport ("VMotionLib")]
	private static extern int VMotionGetControllerCount();

	[DllImport ("VMotionLib")]
	private static extern int VMotionGetYPR_Ptr(uint idx, float[] result, uint timeout);

	[MonoPInvokeCallbackAttribute(typeof(VMotionControllerConnectionCB))]
	protected static void OnVMotionControllerConnectionCB(IntPtr sender, int connected)
	{
		if(OnVMotionConnection != null)
		{
			OnVMotionConnection(((VMOTION_RESULT)connected == VMOTION_RESULT.VMOTION_SUCCESS));
		}
	}
	
	#endregion

	public static bool InitVMotion(string devicePath = "COM5")
	{
		return ((VMOTION_RESULT)VMotionInit(OnVMotionControllerConnectionCB, devicePath) == VMOTION_RESULT.VMOTION_SUCCESS);
	}

	public static bool ShutdownVMotion()
	{
		return ((VMOTION_RESULT)VMotionShutdown() == VMOTION_RESULT.VMOTION_SUCCESS);
	}

	private static float[] data = new float[3];
	public static Vector3 GetYawPitchRoll()
	{
		uint num_controllers = (uint)VMotionGetControllerCount();

		// simply return data from first controller for now
		for(uint i = 0; i < num_controllers; i++)
		{
			if((VMOTION_RESULT)VMotionGetYPR_Ptr(i, data, timeout) == VMOTION_RESULT.VMOTION_SUCCESS)
			{
				return new Vector3(data[0], data[1], data[2]);
			}
		}

		return Vector3.zero;
	}

}
