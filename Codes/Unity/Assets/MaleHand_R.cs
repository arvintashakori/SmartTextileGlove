using System;
using System.IO;
using System.Collections;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using UnityEngine;

public class MaleHand_R : MonoBehaviour
{
    //whole hand
    public GameObject MaleHandRight;


    //palm
    public GameObject BoneArm;
    public GameObject BoneWristRoll;
    public GameObject BoneHand;

    //thumb
    public GameObject Bone003;
    public GameObject Bone004;
    public GameObject Bone005;

    //index
    public GameObject BoneIndexBase;
    public GameObject BoneIndexMid;
    public GameObject BoneIndexEnd;


    //middle finger
    public GameObject BoneMiddleBase;
    public GameObject BoneMiddleMid;
    public GameObject BoneMiddleEnd;

    //pinky finger
    public GameObject BonePinkyBase;
    public GameObject BonePinkyMid;
    public GameObject BonePinkyEnd;

    //ring finger
    public GameObject BoneRingBase;
    public GameObject BoneRingMid;
    public GameObject BoneRingEnd;


    //ports
    public int openposeport = 3000;
    public int pythonport = 8911;
    public int datagrapherport = 12345;
    public int datagrapherrotationinfo = 3005;


    //counters and flags
    public int initflag = -1;
    public bool show_python = true;
    public int dataready = 0;
    public int python_ready = 0;
    public int openapps = 0;
    public int transform = 0;
    public string pins = "";
    public string address = "";
	public int transform_wholehand = 0;
    public int transform_firsttime = 1;
    public int fingerinit = 1;

	// recceived pose parameters
    public string[] datapoints = new string[] { "0.000", "0.000", "0.000", "0.000", "0.000", "0.000", "0.000", "0.000", "0.000", "0.000", "0.000", "0.000", "0.000", "0.000", "0.000", "0.000", "0.000", "0.000", "0.000", "0.000", "0.000", "0.000", "0.000", "0.000", "0.000", "0.000", "0.000", "0.000", "0.000", "0.000" , "0.000", "0.000", "0.000", "0.000", "0.000", "0.000", "0.000", "0.000", "0.000", "0.000", "0.000", "0.000", "0.000", "0.000", "0.000", "0.000", "0.000", "0.000" };
	
	//Pinky rotation values
	public Quaternion Pinky_MCP;
	public Quaternion Pinky_PIP;
	public Quaternion Pinky_DIP;
	
	//Ring rotation values
	public Quaternion Ring_MCP;
	public Quaternion Ring_PIP;
	public Quaternion Ring_DIP;
	
	//Middle rotation values
	public Quaternion Middle_MCP;
	public Quaternion Middle_PIP;
	public Quaternion Middle_DIP;
	
	//Index rotation values
	public Quaternion Index_MCP;
	public Quaternion Index_PIP;
	public Quaternion Index_DIP;
	
	//Thumbs rotation values
	public Quaternion Thumbs_CMC;
	public Quaternion Thumbs_MCP;
	public Quaternion Thumbs_IP;
	
	//Hand rotation values
	public Quaternion Hand;
	

    #region private members 	
    /// <summary> 	
    /// TCPListener to listen for incomming TCP connection 	
    /// requests. 	
    /// </summary> 	
    private TcpListener tcpListener;
    private TcpListener tcpListener1;
    private TcpListener tcpListener2;
    private TcpListener tcpListener3;
    /// <summary> 
    /// Background thread for TcpServer workload. 	
    /// </summary> 	
    private Thread tcpListenerThread;
    private Thread tcpListenerThread1;
    private Thread tcpListenerThread2;
    private Thread tcpListenerThread3;
    /// <summary> 	
    /// Create handle to connected tcp client. 	
    /// </summary> 	
    private TcpClient connectedTcpClient;
    private TcpClient connectedTcpClient1;
    private TcpClient connectedTcpClient2;
    private TcpClient connectedTcpClient3;




    private System.Diagnostics.Process process1;
    private System.Diagnostics.Process process2;
    private System.Diagnostics.Process process3;
    private System.Diagnostics.Process process4;

    #endregion

    public class Debug
    {
        public static void Log(object obj)
        {
            UnityEngine.Debug.Log(System.DateTime.Now.ToString("hh.mm.ss.ffffff") + " : " + obj);

        }
    }


    void FixedUpdate()
    {
        address = Directory.GetCurrentDirectory();
        if (openapps == 0)
        {
            openapps = 4;
            //TexavieBPEV2.0

            //string command1 = "/c cd " + address + "\\Assets\\  &  test.py";
            string command1 = "/c cd " + address + "\\Assets & python simulate.py";
            process1 = new System.Diagnostics.Process();
            System.Diagnostics.ProcessStartInfo startInfo = new System.Diagnostics.ProcessStartInfo();
            if (show_python == true)
            {
                startInfo.WindowStyle = System.Diagnostics.ProcessWindowStyle.Normal;
            }
            else
            {
                startInfo.WindowStyle = System.Diagnostics.ProcessWindowStyle.Hidden;
            }
            startInfo.FileName = "cmd.exe";
            startInfo.Arguments = command1;
            process1.StartInfo = startInfo;
            process1.Start();
        }

        else if ((openapps == 1) && (python_ready == 1))
        {
            openapps = 2;
            //TexavieBPEV2.0

            string command2 = "/c cd " + address + "\\Openpose_Texavie_realtime\\bin\\Debug\\ & Openpose_Texavie.exe";
            process2 = new System.Diagnostics.Process();
            System.Diagnostics.ProcessStartInfo startInfo = new System.Diagnostics.ProcessStartInfo();
            startInfo.WindowStyle = System.Diagnostics.ProcessWindowStyle.Hidden;
            startInfo.FileName = "cmd.exe";
            startInfo.Arguments = command2;
            process2.StartInfo = startInfo;
            process2.Start();


        }

        else if (openapps == 2)
        {
            openapps = 3;
            //Datagrapher

            string command3 = "/c cd " + address + "\\Datagrapher\\bin\\Debug\\  & DataGrapher.exe";
            process3 = new System.Diagnostics.Process();
            System.Diagnostics.ProcessStartInfo startInfo = new System.Diagnostics.ProcessStartInfo();
            startInfo.WindowStyle = System.Diagnostics.ProcessWindowStyle.Hidden;
            startInfo.FileName = "cmd.exe";
            startInfo.Arguments = command3;
            process3.StartInfo = startInfo;
            process3.Start();

        }

        else if ((openapps == 3) && (python_ready == 1))
        {
            openapps = 4;
            //Datagrapher-WPF

            string command4 = "/c cd " + address + "\\DataGrapher-WPF\\CustomDAQ\\CustomDAQ\\bin\\x64\\Debug & CustomDAQ.exe";
            //string command2 = "/c cd C:\\Users\\walker\\Documents\\Texavie\\Git\\DataGrapher-WPF\\CustomDAQ\\CustomDAQ\\bin\\Debug & CustomDAQ.exe";
            process4 = new System.Diagnostics.Process();
            System.Diagnostics.ProcessStartInfo startInfo = new System.Diagnostics.ProcessStartInfo();
            startInfo.WindowStyle = System.Diagnostics.ProcessWindowStyle.Hidden;
            startInfo.FileName = "cmd.exe";
            startInfo.Arguments = command4;
            process4.StartInfo = startInfo;
            process4.Start();

        }

    }



    void Update()
    {
        if (initflag == -1)
        {
            // Machine Learning Server 		
            tcpListenerThread = new Thread(new ThreadStart(ListenForIncommingRequests_python));
            tcpListenerThread.IsBackground = true;
            tcpListenerThread.Start();

            //Datagrapher Server
            tcpListenerThread1 = new Thread(new ThreadStart(ListenForIncommingRequests_datagrapher));
            tcpListenerThread1.IsBackground = true;
            tcpListenerThread1.Start();

            //OpenPose Server
            tcpListenerThread2 = new Thread(new ThreadStart(ListenForIncommingRequests_openpose));
            tcpListenerThread2.IsBackground = true;
            tcpListenerThread2.Start();

            //Datagrapher rotation info Server
            tcpListenerThread3 = new Thread(new ThreadStart(ListenForIncommingRequests_datagrapher_rotation_info));
            tcpListenerThread3.IsBackground = true;
            tcpListenerThread3.Start();

            initflag = 0;
        }



        if (dataready == 1)
        {
            dataready = -1;
            string[] buffer = pins.Split('\n');
            SendMessage(buffer[0]);
            //Debug.Log("Unity sent " + buffer[0] + "to the Python code for the right hand.");
            Debug.Log("bno data:" + buffer[1]);
        }

        if (transform == 1)
        {
			//Point finger values
			Index_MCP = new Quaternion(-1*float.Parse(datapoints[36]), float.Parse(datapoints[37]), float.Parse(datapoints[38]), -1*float.Parse(datapoints[39]));
			Index_PIP = new Quaternion(-1*float.Parse(datapoints[40]), float.Parse(datapoints[41]), float.Parse(datapoints[42]), -1*float.Parse(datapoints[43]));
			Index_DIP = new Quaternion(-1*float.Parse(datapoints[44]), float.Parse(datapoints[45]), float.Parse(datapoints[46]), -1*float.Parse(datapoints[47]));

            // Middle finger values
			Middle_MCP = new Quaternion(-1*float.Parse(datapoints[24]), float.Parse(datapoints[25]), float.Parse(datapoints[26]), -1*float.Parse(datapoints[27]));
			Middle_PIP = new Quaternion(-1*float.Parse(datapoints[28]), float.Parse(datapoints[29]), float.Parse(datapoints[30]), -1*float.Parse(datapoints[31]));
			Middle_DIP = new Quaternion(-1*float.Parse(datapoints[32]), float.Parse(datapoints[33]), float.Parse(datapoints[34]), -1*float.Parse(datapoints[35]));

			// Pinky finger values
			Pinky_MCP = new Quaternion(-1*float.Parse(datapoints[0]), float.Parse(datapoints[1]), float.Parse(datapoints[2]), -1*float.Parse(datapoints[3]));
			Pinky_PIP = new Quaternion(-1*float.Parse(datapoints[4]), float.Parse(datapoints[5]), float.Parse(datapoints[6]), -1*float.Parse(datapoints[7]));
			Pinky_DIP = new Quaternion(-1*float.Parse(datapoints[8]), float.Parse(datapoints[9]), float.Parse(datapoints[10]), -1*float.Parse(datapoints[11]));
			
			// Ring finger values
			Ring_MCP = new Quaternion(-1*float.Parse(datapoints[12]), float.Parse(datapoints[13]), float.Parse(datapoints[14]), -1*float.Parse(datapoints[15]));
			Ring_PIP = new Quaternion(-1*float.Parse(datapoints[16]), float.Parse(datapoints[17]), float.Parse(datapoints[18]), -1*float.Parse(datapoints[19]));
			Ring_DIP = new Quaternion(-1*float.Parse(datapoints[20]), float.Parse(datapoints[21]), float.Parse(datapoints[22]), -1*float.Parse(datapoints[23]));
			
			// Thumbs finger values
			Thumbs_CMC = new Quaternion(-1*float.Parse(datapoints[48]), float.Parse(datapoints[49]), float.Parse(datapoints[50]), -1*float.Parse(datapoints[51]));
			Thumbs_MCP = new Quaternion(-1*float.Parse(datapoints[52]), float.Parse(datapoints[53]), float.Parse(datapoints[54]), -1*float.Parse(datapoints[55]));
			Thumbs_IP = new Quaternion(-1*float.Parse(datapoints[56]), float.Parse(datapoints[57]), float.Parse(datapoints[58]), -1*float.Parse(datapoints[59]));
			
			// Hand values
			Hand = new Quaternion(-1*float.Parse(datapoints[60]), float.Parse(datapoints[61]), float.Parse(datapoints[62]), -1*float.Parse(datapoints[63]));

            //Point finger rotation
			BoneIndexBase.transform.localRotation = Quaternion.Inverse(Hand) * Index_MCP;
			BoneIndexMid.transform.localRotation = Quaternion.Inverse(Index_MCP) * Index_PIP;
			BoneIndexEnd.transform.localRotation = Quaternion.Inverse(Index_PIP) * Index_DIP;

            // Middle finger rotation
			BoneMiddleBase.transform.localRotation = Quaternion.Inverse(Hand) * Middle_MCP;
			BoneMiddleMid.transform.localRotation = Quaternion.Inverse(Middle_MCP) * Middle_PIP;
			BoneMiddleEnd.transform.localRotation = Quaternion.Inverse(Middle_PIP) * Middle_DIP;

            //Pinky finger rotation
			BonePinkyBase.transform.localRotation = Quaternion.Inverse(Hand) * Pinky_MCP;
			BonePinkyMid.transform.localRotation = Quaternion.Inverse(Pinky_MCP) * Pinky_PIP;
			BonePinkyEnd.transform.localRotation = Quaternion.Inverse(Pinky_PIP) * Pinky_DIP;

            //Ring finger rotation
			BoneRingBase.transform.localRotation = Quaternion.Inverse(Hand) * Ring_MCP;
			BoneRingMid.transform.localRotation = Quaternion.Inverse(Ring_MCP) * Ring_PIP;
			BoneRingEnd.transform.localRotation = Quaternion.Inverse(Ring_PIP) * Ring_DIP;

            //Thumb finger rotation
			Bone003.transform.localRotation = Quaternion.Inverse(Hand) * Thumbs_CMC;
			Bone004.transform.localRotation = Quaternion.Inverse(Thumbs_CMC) * Thumbs_MCP;
			Bone005.transform.localRotation = Quaternion.Inverse(Thumbs_MCP) * Thumbs_IP;

            //whole hand rotation
			BoneArm.transform.localRotation = Hand;
			
            transform = 0;
        }



        if (transform_wholehand == 1)
        {

            transform_wholehand = 0;
        }

    }

    void OnApplicationQuit()
    {
        try
        {
            tcpListenerThread.Suspend();
            Debug.Log("Machine Learning Server for right hand is Stopped.");
        }
        catch
        {
            Debug.Log("Could not stop Machine Learning Server for right hand.");
        }

        try
        {
            tcpListenerThread1.Suspend();
            Debug.Log("DataGrapher Server for Right right is Stopped.");
        }
        catch
        {
            Debug.Log("Could not stop DataGrapher Server for right hand.");
        }

        try
        {
            tcpListenerThread2.Suspend();
            Debug.Log("OpenPose Server for right hand is Stopped.");
        }
        catch
        {
            Debug.Log("Could not stop OpenPose Server for right hand.");
        }
        try
        {
            tcpListenerThread3.Suspend();
            Debug.Log("Datagrapher rotation info Server for right hand is Stopped.");
        }
        catch
        {
            Debug.Log("Could not stop Datagrapher rotation info Server for right hand.");
        }
        try
        {
            process1.Kill();
            process2.Kill();
            process3.Kill();
            process4.Kill();
            Debug.Log("CMDs for right hand are Stopped.");
        }
        catch
        {
            Debug.Log("Could not stop CMDs for right hand.");
        }
    }

    /// <summary> 	
    /// Runs in background TcpServerThread; Handles incomming TcpClient requests 	
    /// </summary> 	
    private void ListenForIncommingRequests_python()
    {
        try
        {
            // Create listener on localhost port 8910. 			
            tcpListener = new TcpListener(IPAddress.Parse("127.0.0.1"), pythonport);
            tcpListener.Start();
            Debug.Log("Python Server for right hand is listening");
            Byte[] bytes = new Byte[1024];
            while (true)
            {
                using (connectedTcpClient = tcpListener.AcceptTcpClient())
                {
                    // Get a stream object for reading 					
                    using (NetworkStream stream = connectedTcpClient.GetStream())
                    {
                        int length;
                        // Read incomming stream into byte arrary. 						
                        while ((length = stream.Read(bytes, 0, bytes.Length)) != 0)
                        {
                            var incommingData = new byte[length];
                            Array.Copy(bytes, 0, incommingData, 0, length);
                            // Convert byte array to string message. 							
                            string clientMessage = Encoding.ASCII.GetString(incommingData);
                            if (clientMessage == "r")
                            {
                                python_ready = 1;
                                Debug.Log("Python Server for left hand is ready");
                            }
                            else
                            {
                                if (fingerinit == 1)
                                {
                                    fingerinit = 0;
                                    datapoints = clientMessage.Split(',');
                            
                                    if (dataready == -1)
                                    {
                                        dataready = 0;
                                    }
                                }
                                else
                                {
                                    datapoints = clientMessage.Split(',');
                                    transform = 1;
                                    if (dataready == -1)
                                    {
                                        dataready = 0;
                                    }
                                    //Debug.Log("left hand Python client message received as: " + clientMessage);
                                }
                            }
                        }
                    }
                }
            }
        }
        catch (SocketException socketException)
        {
            Debug.Log("Python SocketException for right hand: " + socketException.ToString());
        }
    }



    private void ListenForIncommingRequests_openpose()
    {
        try
        {
            // Create listener on localhost port 3000. 			
            tcpListener1 = new TcpListener(IPAddress.Parse("127.0.0.1"), openposeport);
            tcpListener1.Start();
            Debug.Log("OpenPose Server for Right hand is listening");
            Byte[] bytes = new Byte[1024];
            while (true)
            {
                using (connectedTcpClient = tcpListener1.AcceptTcpClient())
                {
                    // Get a stream object for reading 					
                    using (NetworkStream stream = connectedTcpClient.GetStream())
                    {
                        int length;
                        // Read incomming stream into byte arrary. 						
                        while ((length = stream.Read(bytes, 0, bytes.Length)) != 0)
                        {
                            var incommingData = new byte[length];
                            Array.Copy(bytes, 0, incommingData, 0, length);
                            // Convert byte array to string message. 							
                            string clientMessage = Encoding.ASCII.GetString(incommingData);
                            
                                datapoints = clientMessage.Split(',');
                                transform = 1;


                            if (dataready == -1)
                            {
                                dataready = 0;
                            }
                            //Debug.Log("Right hand OpenPose client message received as: " + clientMessage);
                        }
                    }
                }
            }
        }
        catch (SocketException socketException)
        {
            Debug.Log("OpenPose SocketException for Right hand: " + socketException.ToString());
        }
    }




    private void ListenForIncommingRequests_datagrapher()
    {
        try
        {
            // Create listener on localhost port 12345. 			
            tcpListener2 = new TcpListener(IPAddress.Parse("127.0.0.1"), datagrapherport);
            tcpListener2.Start();
            Debug.Log("DataGrapher Server for Right hand is listening");
            Byte[] bytes = new Byte[1024];
            while (true)
            {
                using (connectedTcpClient1 = tcpListener2.AcceptTcpClient())
                {
                    // Get a stream object for reading 					
                    using (NetworkStream stream = connectedTcpClient1.GetStream())
                    {
                        int length;
                        // Read incomming stream into byte arrary. 						
                        while ((length = stream.Read(bytes, 0, bytes.Length)) != 0)
                        {
                            var incommingData = new byte[length];
                            Array.Copy(bytes, 0, incommingData, 0, length);
                            // Convert byte array to string message. 							
                            string clientMessage = Encoding.ASCII.GetString(incommingData);
                            pins = clientMessage;
                            if (dataready == 0)
                            {
                                dataready = 1;
                            }
                            //Debug.Log("Right handDataGrapher client message received as: " + clientMessage);
                        }
                    }
                }
            }
        }
        catch (SocketException socketException)
        {
            Debug.Log("DataGrapher SocketException for Right hand " + socketException.ToString());
        }
    }


    private void ListenForIncommingRequests_datagrapher_rotation_info()
    {
        try
        {
            // Create listener on localhost port 3005. 			
            tcpListener3 = new TcpListener(IPAddress.Parse("127.0.0.1"), datagrapherrotationinfo);
            tcpListener3.Start();
            Debug.Log("Datagrapher rotation info Server for Left hand is listening");
            Byte[] bytes = new Byte[1024];
            while (true)
            {
                using (connectedTcpClient3 = tcpListener3.AcceptTcpClient())
                {
                    // Get a stream object for reading 					
                    using (NetworkStream stream = connectedTcpClient3.GetStream())
                    {
                        int length;
                        // Read incomming stream into byte arrary. 						
                        while ((length = stream.Read(bytes, 0, bytes.Length)) != 0)
                        {
                            var incommingData3 = new byte[length];
                            Array.Copy(bytes, 0, incommingData3, 0, length);
                            // Convert byte array to string message. 							
                            string clientMessage3 = Encoding.ASCII.GetString(incommingData3);

                                datapoints = clientMessage3.Split(',');
                                transform_wholehand = 1;

                            //Debug.Log("Left hand datagraoher rotation info client message received as: " + clientMessage3);
                        }
                    }
                }
            }
        }
        catch (SocketException socketException)
        {
            Debug.Log("OpenPose SocketException for Left hand: " + socketException.ToString());
        }
    }



    /// <summary> 	
    /// Send message to client using socket connection. 	
    /// </summary> 	
    private void SendMessage(string data)
    {
        if (connectedTcpClient == null)
        {
            return;
        }

        try
        {
            // Get a stream object for writing. 			
            NetworkStream stream = connectedTcpClient.GetStream();
            if (stream.CanWrite)
            {
                string serverMessage = data;
                // Convert string message to byte array.                 
                byte[] serverMessageAsByteArray = Encoding.ASCII.GetBytes(serverMessage);
                // Write byte array to socketConnection stream.               
                stream.Write(serverMessageAsByteArray, 0, serverMessageAsByteArray.Length);
                //Debug.Log("Server sent his message - should be received by client");
            }
        }
        catch (SocketException socketException)
        {
            Debug.Log("DataGrapher Socket exception for Right hand: " + socketException);
        }
    }



}
