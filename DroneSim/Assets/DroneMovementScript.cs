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
	// Private
	private float desiredYRotation;
	private float liftVelocity;
	private float rotationAmount = 2.5f;
	private float rotationYVelocity;
	private float tiltForward = 0;
	private float tiltForwardVelocity;
	private float tiltSideways = 0;
	private float tiltSidewaysVelocity;
	private Vector3 velocitySmoothDamp; // This is for speed clamping.
	// Public
	public float airDensity = 1.225f; // Air density in kg/m^3
	[HideInInspector] public float currentYRotation; // This is for drone rotation.
	public bool isRotatable = false; // This controls whether the drone can rotate.
	public float lift; // This is the net force from all propellers.
	public float liftCoefficient = 0.2f; // This is a constant that is normally determined experimentally.
	public float propellerArea = 0.03f; // This is the area of the propeller wing in square meters.
	public float propellerDiameter = 10; // This is the propeller diameter (tip-to-tip) in inches.
	public float RPMMax = 7170; // This is 80% of the maximum RPM.
	public float RPMMin = 5610; // This is the slowest RPM allowed for the propellers.
	public float tiltAngleForward = 20; // This is the forward/backward angle tilt of the propeller in degrees.
	public float tiltAngleRight = 20; // This is the sideways angle tilt of the propeller in degrees.
	Rigidbody drone;
	
	// This function is called only once, after all objects are initialized.
	void Awake() 
	{
		drone = GetComponent<Rigidbody>();
	}
	
	// This function is called every fixed frame-rate frame.
	// The default time between calls is 0.02 seconds.
	void FixedUpdate()
	{
		MovementUpDown();
		MovementLeftRight();
		MovementForwardBackward();
		Rotate();
		ClampingSpeedValues();
		drone.AddRelativeForce( Vector3.up * lift );
		drone.rotation = Quaternion.Euler( new Vector3( tiltForward, currentYRotation, tiltSideways ) );
	}
	
	// Simulates upward and downward movement
	void MovementUpDown()
	{		
		if ( Input.GetKey( KeyCode.I ) && !Input.GetKey( KeyCode.K ) )
		{
			liftVelocity = SetLiftVelocity( RPMMax );
			lift = 4 * SetLift( liftVelocity );
		}
		else if ( Input.GetKey( KeyCode.K ) && !Input.GetKey( KeyCode.I ) )
		{
			liftVelocity = SetLiftVelocity( RPMMin );
			lift = 4 * SetLift( liftVelocity );
		}
		else 
		{
			// Hovers when stationary
			lift = 98.1f;
		}
	}
	
	// Simulates forward and backward movement
	void MovementForwardBackward()
	{
		if ( Mathf.Abs( Input.GetAxis( "Vertical" ) ) > 0.2f )
		{
			tiltForward = Mathf.SmoothDamp( tiltForward, tiltAngleForward * Input.GetAxis("Vertical"), ref tiltForwardVelocity, 0.1f );
			lift = SetMovementForce( tiltAngleForward );
		}
		else 
		{
			tiltForward = Mathf.SmoothDamp( tiltForward, 0, ref tiltForwardVelocity, 0.1f );
		}
		
	}
	
	// Simulates left and right movement
	void MovementLeftRight()
	{
		if ( Mathf.Abs( Input.GetAxis( "Horizontal" ) ) > 0.2f )
		{
			tiltSideways = Mathf.SmoothDamp( tiltSideways, -1 * tiltAngleRight * Input.GetAxis( "Horizontal" ), ref tiltSidewaysVelocity, 0.1f );
			lift = SetMovementForce(tiltAngleRight);
		}
		else 
		{
			tiltSideways = Mathf.SmoothDamp( tiltSideways, 0, ref tiltSidewaysVelocity, 0.1f );
		}
	}
	
	// Simulates left and right rotational movement
	void Rotate()
	{
		if ( !isRotatable )
		{
			return;
		}
		if ( Input.GetKey( KeyCode.J ) ) 
		{
			desiredYRotation -= rotationAmount;
		}
		if ( Input.GetKey( KeyCode.L ) ) 
		{
			desiredYRotation += rotationAmount;
		}
		currentYRotation = Mathf.SmoothDamp( currentYRotation, desiredYRotation, ref rotationYVelocity, 0.25f );
	} 
	
	// Clamps movement speeds
	void ClampingSpeedValues()
	{
		if ( Mathf.Abs( Input.GetAxis( "Vertical" ) ) > 0.2f || Mathf.Abs( Input.GetAxis( "Horizontal" ) ) > 0.2f )
		{
			drone.velocity = Vector3.ClampMagnitude( drone.velocity, Mathf.Lerp( drone.velocity.magnitude, 30.0f, Time.deltaTime * 5f ) );
		}
		else 
		{
			drone.velocity = Vector3.SmoothDamp( drone.velocity, Vector3.zero, ref velocitySmoothDamp, 0.95f );
		}
	}
	
	float SetMovementForce(float tiltAmountDirection)
	{
		return lift * Mathf.Sin( ( 90 - tiltAmountDirection ) * Mathf.PI / 180 );
	}
	
	float SetLift(float velocity)
	{
		return liftCoefficient * ( airDensity * Mathf.Pow(velocity, 2) / 2 ) * propellerArea;
	}	

	float SetLiftVelocity(float RPM)
	{
		return RPM * ConvertUnitsDiameter() * Mathf.PI / 60; // (Revolution/min * 2pi[radians]/revolution * 1min/60sec * diameter[meters]/2) === (meter/second)
	}

	float ConvertUnitsDiameter()
	{
		return propellerDiameter * 0.0254f; // Conversion from inch to meters
	}
}
