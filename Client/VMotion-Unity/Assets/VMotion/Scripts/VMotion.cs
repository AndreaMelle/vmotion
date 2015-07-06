using UnityEngine;
using System.Collections;

public class VMotion : MonoBehaviour
{

	bool isConnected = false;

	// Use this for initialization
	void Start ()
	{
		DontDestroyOnLoad (this);
		// this will cause error if recalled. How to allow for re-init?
		// will a Unity scee reload a plugin DLL??
		VMotionInternals.OnVMotionConnection += OnConnection;
		VMotionInternals.InitVMotion ();
	}
	
	// Update is called once per frame
	void Update ()
	{
		if(isConnected)
		{
			Vector3 ypr = VMotionInternals.GetYawPitchRoll();
		}
	}

	public void OnConnection(bool success)
	{
		isConnected = success;
		if(isConnected)
		{
			Debug.Log ("VMotion connection succesful.");
		}
		else
		{
			Debug.Log ("VMotion connection failed.");
		}
	}

	void OnApplicationQuit()
	{
		VMotionInternals.InitVMotion ();
	}


}
