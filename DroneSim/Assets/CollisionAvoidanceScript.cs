/*
 * Drone Simulation - Collision Avoidance Script
 * CS 179J - Spring 2019
 * Joshua Riley
 * Adriel Bustamente
 * Colton Vosburg
 * Jonathan Woolf
*/
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class CollisionAvoidanceScript : MonoBehaviour
{
	// Private
	
	// Public
    Rigidbody drone;
	
    void Awake() 
	{
		drone = GetComponent<Rigidbody>();
	}

    void OnCollisionEnter( Collision col )
	{
		// Disable movement in direction of collision
		if( col.gameObject.name != "Package" )
		{
			drone.velocity = Vector3.zero;
		}
	}
}