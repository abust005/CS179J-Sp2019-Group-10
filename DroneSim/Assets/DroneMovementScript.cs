/*
 * Drone Simulation Script
 * CS 179J - Spring 2019
 * Joshua Riley
 * Adriel Bustamente
 * Colton Vosburg
 * Jonathan Woolf
*/
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class DroneMovementScript : MonoBehaviour
{
    
	Rigidbody drone;
	// Total upward force from propellers
	public float lift;
	// Propeller forces
	public float maxLift = 300;
	public float minLift = 60;
	public float forceAdjustmentA = 135; // Combination of forward/backward and left/right movement
	public float forceAdjustmentB = 120; // forceAdjustmentA && rotational movement
	public float forceAdjustmentC = 150; // forceAdjustmentA && up
	public float forceAdjustmentD = 110;  // forceAdjustmentB && down
	public float forceAdjustmentE = 140;   // forceAdjustmentB && up movement
	public float forceAdjustmentF = 105;   // forceAdjustmentB && down movement
	// Drone movement speeds
	public float movementAmount = 200.0f;
	// Associated movement tilts
	public float tiltAmountForward = 20;
	public float tiltAmountSideways = 20;
	private float tiltForward = 0;
	private float tiltSideways = 0;
	private float tiltForwardVelocity;
	private float tiltAmountVelocity;
	// Drone rotation
	private float desiredYRotation;
	[HideInInspector] public float currentYRotation;
	private float rotateAmountByKeys = 2.5f;
	private float rotationYVelocity;
	// For speed clamping
	private Vector3 velocitySmoothDamp;
	
	// Called only once, after all objects are initialized
	void Awake() {
		drone = GetComponent<Rigidbody>();
	}
	
	// Called every fixed frame-rate frame; Default time between calls: 0.02 seconds
	void FixedUpdate(){
		MovementUpDown();
		MovementLeftRight();
		MovementForwardBackward();
		Rotation();
		ClampingSpeedValues();
		AdjustLift();
		RegulateForce();
		
		drone.AddRelativeForce(Vector3.up * lift);
		drone.rotation = Quaternion.Euler(
			new Vector3(tiltForward, currentYRotation, tiltSideways)
		);
	}
	
	// Simulates upward and downward movement
	void MovementUpDown(){		
		if (Input.GetKey(KeyCode.I)){
			lift = maxLift;
		}
		else if (Input.GetKey(KeyCode.K)){
			lift = minLift;
		}
		else if (!Input.GetKey(KeyCode.I) && !Input.GetKey(KeyCode.K) && (Mathf.Abs(Input.GetAxis("Vertical")) < 0.2f && Mathf.Abs(Input.GetAxis("Horizontal")) < 0.2f)){
			// Hovers when stationary
			lift = 98.1f;
		}
		
	}
	
	// Simulates forward and backward movement
	void MovementForwardBackward(){
		if (Mathf.Abs(Input.GetAxis("Vertical")) > 0.2f){
			drone.AddRelativeForce(Vector3.forward * Input.GetAxis("Vertical") * movementAmount);
			tiltForward = Mathf.SmoothDamp(tiltForward, tiltAmountForward * Input.GetAxis("Vertical"), ref tiltForwardVelocity, 0.1f);
			
		}
		else {
			tiltForward = Mathf.SmoothDamp(tiltForward, 0, ref tiltForwardVelocity, 0.1f);
		}
		
	}
	
	// Simulates left and right movement
	void MovementLeftRight(){
		if (Mathf.Abs(Input.GetAxis("Horizontal")) > 0.2f){
			drone.AddRelativeForce(Vector3.right * Input.GetAxis("Horizontal") * movementAmount);
			tiltSideways = Mathf.SmoothDamp(tiltSideways, -1 * tiltAmountSideways * Input.GetAxis("Horizontal"), ref tiltAmountVelocity, 0.1f);
		}
		else {
			tiltSideways = Mathf.SmoothDamp(tiltSideways, 0, ref tiltAmountVelocity, 0.1f);
		}
	}
	
	// Simulates left and right rotational movement
	void Rotation(){
		if(Input.GetKey(KeyCode.J)){
			desiredYRotation -= rotateAmountByKeys;
		}
		if (Input.GetKey(KeyCode.L)){
			desiredYRotation += rotateAmountByKeys;
		}
		
		currentYRotation = Mathf.SmoothDamp(currentYRotation, desiredYRotation, ref rotationYVelocity, 0.25f);
	} 
	
	// Clamps movement speeds
	void ClampingSpeedValues(){
		if (Mathf.Abs(Input.GetAxis("Vertical")) > 0.2f || Mathf.Abs(Input.GetAxis("Horizontal")) > 0.2f){
			drone.velocity = Vector3.ClampMagnitude(drone.velocity, Mathf.Lerp(drone.velocity.magnitude, 30.0f, Time.deltaTime * 5f));
		}
		else {
			drone.velocity = Vector3.SmoothDamp(drone.velocity, Vector3.zero, ref velocitySmoothDamp, 0.95f);
		}
	}
	
	// Adjust the lift for when the drone is tilted
	void AdjustLift(){
		if ((Mathf.Abs(Input.GetAxis("Vertical")) > 0.2f) || (Mathf.Abs(Input.GetAxis("Horizontal")) > 0.2f)){
			if (!Input.GetKey(KeyCode.I) && !Input.GetKey(KeyCode.K) && !Input.GetKey(KeyCode.J) && !Input.GetKey(KeyCode.L)){
				drone.velocity = new Vector3(drone.velocity.x, Mathf.Lerp(drone.velocity.y, 0, Time.deltaTime*5), drone.velocity.z);
				lift = forceAdjustmentA;
			}
			else if (!Input.GetKey(KeyCode.I) && !Input.GetKey(KeyCode.K) && (Input.GetKey(KeyCode.J) || Input.GetKey(KeyCode.L))){
				drone.velocity = new Vector3(drone.velocity.x, Mathf.Lerp(drone.velocity.y, 0, Time.deltaTime*5), drone.velocity.z);
				lift = forceAdjustmentB;
			}
			else if (Input.GetKey(KeyCode.I) && (Input.GetKey(KeyCode.J) || Input.GetKey(KeyCode.L))){
				drone.velocity = new Vector3(drone.velocity.x, Mathf.Lerp(drone.velocity.y, 0, Time.deltaTime*5), drone.velocity.z);
				lift = forceAdjustmentE;
			}
			else if (Input.GetKey(KeyCode.K) && (Input.GetKey(KeyCode.J) || Input.GetKey(KeyCode.L))){
				drone.velocity = new Vector3(drone.velocity.x, Mathf.Lerp(drone.velocity.y, 0, Time.deltaTime*5), drone.velocity.z);
				lift = forceAdjustmentF;
			}
			else if (Input.GetKey(KeyCode.I) && !Input.GetKey(KeyCode.K)){
				lift = forceAdjustmentC;
			}
			else if (!Input.GetKey(KeyCode.I) && Input.GetKey(KeyCode.K)){
				lift = forceAdjustmentD;
			}
		}
	}
	
	void RegulateForce(){
		if (forceAdjustmentA > maxLift){
			forceAdjustmentA = maxLift;
		}
		else if (forceAdjustmentA < minLift){
			forceAdjustmentA = minLift;
		}
		if (forceAdjustmentB > maxLift){
			forceAdjustmentB = maxLift;
		}
		else if (forceAdjustmentB < minLift){
			forceAdjustmentB = minLift;
		}
		if (forceAdjustmentC > maxLift){
			forceAdjustmentC = maxLift;
		}
		else if (forceAdjustmentC < minLift){
			forceAdjustmentC = minLift;
		}
		if (forceAdjustmentD > maxLift){
			forceAdjustmentD = maxLift;
		}
		else if (forceAdjustmentD < minLift){
			forceAdjustmentD = minLift;
		}
		if (forceAdjustmentE > maxLift){
			forceAdjustmentE = maxLift;
		}
		else if (forceAdjustmentE < minLift){
			forceAdjustmentE = minLift;
		}
		if (forceAdjustmentF > maxLift){
			forceAdjustmentF = maxLift;
		}
		else if (forceAdjustmentF < minLift){
			forceAdjustmentF = minLift;
		}
	}
	
}
