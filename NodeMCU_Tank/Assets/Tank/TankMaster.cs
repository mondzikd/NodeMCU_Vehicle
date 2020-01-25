using System;
using System.Collections;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;
using UnityEngine;
using UnityEngine.UI;

public class TankMaster : MonoBehaviour
{
    public InputField udpInput;
    public InputField ipInput;
    public Joystick rightJoystick;
    public Joystick leftJoystick;

    private string ipAddress;
    private int udpPort;

    private UdpClient udpClient = new UdpClient();
    private IPEndPoint remoteEndpoint;

    private byte[] packet = new byte[6] {0x7b, 0x0, 0x7f, 0x7f, 0x01, 0x01};

    private int counter = 0;
    private int ratio = 30;

    // Start is called before the first frame update
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        counter = (counter + 1) % ratio; 
        if (remoteEndpoint == null || counter != 0) return;

        int rightWheel = 127 + Convert.ToInt32(rightJoystick.Vertical * 127);
        int leftWheel = 127 + Convert.ToInt32(leftJoystick.Vertical * 127);
        int softStop = (rightJoystick.Vertical < 0.1 && leftJoystick.Vertical < 0.1) ? 1 : 0;
        packet[2] = (byte) leftWheel;
        packet[3] = (byte) rightWheel;
        packet[4] = packet[5] = (byte) softStop;

        udpClient.Send(packet, 6, remoteEndpoint);
    }


    public void OnClick()
    {
        try
        {
            ipAddress = ipInput.text;
            udpPort = int.Parse(udpInput.text);

            remoteEndpoint = new IPEndPoint(IPAddress.Parse(ipAddress), udpPort);
        }
        catch (Exception e) { }

    }
}
