using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PropellerSpinScript : MonoBehaviour
{
	
	private Transform bladeA;
	private Transform bladeB;
	private Transform bladeC;
	private Transform bladeD;
	Rigidbody A;
	Rigidbody B;
	Rigidbody C;
	Rigidbody D;
	private int rotationA = 0;
	private int rotationB = 0;
	private int rotationC = 0;
	private int rotationD = 0;
	
    // Start is called before the first frame update
    void Awake()
    {
		bladeA = GameObject.FindGameObjectWithTag("TopLeftBlade").transform;
		bladeB = GameObject.FindGameObjectWithTag("TopRightBlade").transform;
		bladeC = GameObject.FindGameObjectWithTag("BottomLeftBlade").transform;
		bladeD = GameObject.FindGameObjectWithTag("BottomRightBlade").transform;
		A = bladeA.GetComponent<Rigidbody>();
		B = bladeB.GetComponent<Rigidbody>();
		C = bladeC.GetComponent<Rigidbody>();
		D = bladeD.GetComponent<Rigidbody>();
    }

	void FixedUpdate(){
		Rotate();
		A.rotation = Quaternion.Euler(new Vector3(0, rotationA, 0));
		B.rotation = Quaternion.Euler(new Vector3(0, rotationB, 0));
		C.rotation = Quaternion.Euler(new Vector3(0, rotationC, 0));
		D.rotation = Quaternion.Euler(new Vector3(0, rotationD, 0));
	}
	
	void Rotate(){
		rotationA = (rotationA >= 360) ? 0 : rotationA + 20;
		rotationB = (rotationB <= 0) ? 360 : rotationB - 20;
		rotationC = (rotationC >= 360) ? 0 : rotationC + 20;
		rotationD = (rotationD <= 0) ? 360 : rotationD - 20;
	}
}
