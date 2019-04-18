using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class CameraFollowScript : MonoBehaviour
{
    private Transform drone;
	private Vector3 velocityCameraFollow;
	public Vector3 behindPosition = new Vector3(0, 50, -30);
	public float cameraAngleX = 20;
	
	void Awake(){
		drone = GameObject.FindGameObjectWithTag("Player").transform;
	}

	void FixedUpdate(){
		transform.position = Vector3.SmoothDamp(transform.position, drone.transform.TransformPoint(behindPosition) + Vector3.up, ref velocityCameraFollow, 0.1f);
		transform.rotation = Quaternion.Euler(new Vector3(cameraAngleX, drone.GetComponent<DroneMovementScript>().currentYRotation, 0));
	}
}
