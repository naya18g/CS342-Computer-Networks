#include <fstream>
#include "ns3/core-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/config-store-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/gnuplot-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/gnuplot.h"
#define nme std::

using namespace ns3;
template<class t> using sg=string;

NS_LOG_COMPONENT_DEFINE("wiredTCP_Simulation");

class my_application : public Application
{

public:
	my_application ();
	virtual ~my_application();

	void Setup(Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate datarate);

private:

	virtual void StartApplication (void);
  	virtual void StopApplication (void);
  	void ScheduleTx (void);
  	void SendPacket (void);
  	Ptr<Socket> ms ;
  	Address m_peer;
  	uint32_t m_size;
  	uint32_t mnp;
  	DataRate m_rate;
  	EventId event;
  	bool m_is_on;
  	uint32_t m_pp;
};

// constructor
my_application::my_application ()
  : 
    ms  (0), 
    m_peer (), 
    m_size (0), 
    mnp (0), 
    m_rate (0), 
    event (), 
    m_is_on (false), 
    m_pp (0)
my_application::~my_application(){ ms  = 0;}

void my_application::Setup(Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate datarate){
	ms  = socket;
	m_peer = address;
	m_size = packetSize;
	mnp = nPackets;
	m_rate = datarate;
}

void my_application::StartApplication() {
	m_is_on = true;
	m_pp = 0;
	ms ->Bind();
	ms ->Connect(m_peer);
	SendPacket();
}

void my_application::StopApplication() {
	m_is_on = false;

	if(event.IsRunning()){
		Simulator::Cancel(event);
	}
	if(ms ){
		ms ->Close();
	}
}

void my_application::SendPacket() {
	Ptr<Packet> packet = Create<Packet>(m_size);
	ms ->Send(packet);

	if(++m_pp < mnp){
		ScheduleTx();
	}
}

void 
my_application::ScheduleTx (void)
{
  if (m_is_on)
    {
      Time tNext (Seconds (m_size * 8 / static_cast<double> (m_rate.GetBitRate ())));
      event = Simulator::Schedule (tNext, &my_application::SendPacket, this);
    }
}

int main(int argc, char *argv[]) {

	LogComponentEnable("wiredTCP_Simulation", LOG_LEVEL_INFO);
// Setup GNU Plot interface
	
	nme sg pt1               = "Fairness Index vs Packet Size";
  	nme sg dt1               = "Fairness Index Data";
 //setting up Fairness plot
  	nme sg ff          = "wiredTCPfairness";
	nme sg fg        = ff + ".png";
	nme sg fp            = ff + ".plt";

	Gnuplot plot1 (fg);
	plot1.SetTitle (pt1);
 //graphics file is created by plot file when we run Gnuplotto create the final plot in a PNG file.
	plot1.SetTerminal ("png");
	plot1.SetLegend ("Packet Size", "Fairness Index");
	plot1.AppendExtra ("set xrange [0:1550]");
	// Instantiate the dataset, set its title, and make the points be plotted along with connecting lines.
	Gnuplot2dDataset fairnessDataset;
	fairnessDataset.SetTitle (dt1);
  	fairnessDataset.SetStyle (Gnuplot2dDataset::LINES_POINTS);

  	nme sg plotTitle2               = "Throughput vs Packet Size";
  	nme sg dataTitle2               = "Throughput Data";

	nme sg throughputFile           = "wiredTCPthroughput";
	nme sg throughputGraphics       = throughputFile + ".png";
	nme sg throughputPlot           = throughputFile + ".plt";

	Gnuplot plot2 (throughputGraphics);
	plot2.SetTitle (plotTitle2);
	// Make the graphics file, which the plot file will create when it is used with Gnuplot, be a PNG file.
	plot2.SetTerminal ("png");
	plot2.SetLegend ("Packet Size", "Throughput");
	plot2.AppendExtra ("set xrange [0:1550]");
	// Instantiate the dataset, set its title, and make the points be plotted along with connecting lines.
	Gnuplot2dDataset throughputDataset;
	throughputDataset.SetTitle (dataTitle2);
	throughputDataset.SetStyle (Gnuplot2dDataset::LINES_POINTS);

//TODO:: loop over different packet sizes
	nme vector<uint32_t> packetSizes = {40, 44, 48, 52, 60, 250, 300, 552, 576, 628, 1420, 1500};
	uint32_t nPackets = 100;

	for(auto it : packetSizes) {
		uint32_t packetSize = it;
	// Setup the netdevices
		// create the nodes
		Ptr<Node> n2 = CreateObject<Node> ();
		Ptr<Node> r1 = CreateObject<Node> ();
		Ptr<Node> r2 = CreateObject<Node> ();
		Ptr<Node> n3 = CreateObject<Node> ();

	// Setup connection media and the physical layer
		// container for each link
		NodeContainer n2r1 = NodeContainer(n2,r1);
		NodeContainer r1r2 = NodeContainer(r1,r2);
		NodeContainer r2n3 = NodeContainer(r2,n3);
		NodeContainer all = NodeContainer(n2,r1,r2,n3);

		// configure p2p links
		PointToPointHelper p2p_n2r1;
		p2p_n2r1.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
		p2p_n2r1.SetChannelAttribute("Delay", StringValue("20ms"));
		NetDeviceContainer device_n2r1;
		device_n2r1 = p2p_n2r1.Install(n2r1);

		PointToPointHelper p2p_r1r2;
		// p2p_r1r2.SetQueue("ns3::DropTailQueue","Mode",EnumValue (DropTailQueue::QUEUE_MODE_BYTES), "MaxBytes", UintegerValue(2048));
		// Droptails Queues are installed by default at the device level.
		p2p_r1r2.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
		p2p_r1r2.SetChannelAttribute("Delay", StringValue("50ms"));
		NetDeviceContainer device_r1r2;
		device_r1r2 = p2p_r1r2.Install(r1r2);

		PointToPointHelper p2p_r2n3;
		p2p_r2n3.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
		p2p_r2n3.SetChannelAttribute("Delay", StringValue("20ms"));
		NetDeviceContainer device_r2n3;
		device_r2n3 = p2p_r2n3.Install(r2n3);

	// Setup the protocol stack
		// install internet stack
		NS_LOG_INFO("Install Internet Stack");
		InternetStackHelper stack;
		stack.Install(all);

	// Setup the Network layer
		// assign IP address to the nodes
		Ipv4AddressHelper ipaddress;
		ipaddress.SetBase("10.0.1.0", "255.255.255.0");
		Ipv4InterfaceContainer Ipinterface_n2r1 = ipaddress.Assign(device_n2r1);
		
		ipaddress.SetBase("10.0.2.0", "255.255.255.0");
		Ipv4InterfaceContainer Ipinterface_r1r2 = ipaddress.Assign(device_r1r2);
		
		ipaddress.SetBase("10.0.3.0", "255.255.255.0");
		Ipv4InterfaceContainer Ipinterface_r2n3 = ipaddress.Assign(device_r2n3);

		nme cout << "r1 ipaddr: " << Ipinterface_r1r2.GetAddress(0) << "  r2 ipaddr: " << Ipinterface_r1r2.GetAddress(1) << nme endl;
		nme cout << "n2 ipaddr: " << Ipinterface_n2r1.GetAddress(0) << "  r1 ipaddr: " << Ipinterface_n2r1.GetAddress(1) << nme endl;
		nme cout << "n3 ipaddr: " << Ipinterface_r2n3.GetAddress(0) << "  n3 ipaddr: " << Ipinterface_r2n3.GetAddress(1) << nme endl;

	// Populate the routing tables
		Ipv4GlobalRoutingHelper routingHelper;
		routingHelper.PopulateRoutingTables();

	// Setup the Application layer
		// creating tcp packetsink for the reciever on n3
		uint16_t port = 4200;
		Address sinkAddress(InetSocketAddress(Ipinterface_r2n3.GetAddress(1),port));
		PacketSinkHelper sinkHelper("ns3::TcpSocketFactory", sinkAddress);
		// Install a packetsink app in the application layer of n3
		ApplicationContainer sinkApp = sinkHelper.Install(n3);
		sinkApp.Start(Seconds(1.0));
		sinkApp.Stop(Seconds(20.0));

	// Setup a tcp sender application on n2
		// create apps for each tcp type
		nme vector<nme string> tcptypes = {"ns3::TcpVegas", "ns3::TcpVeno", "ns3::TcpWestwood"};
		for(int i = 0 ; i < 3 ; i++) {
			Ptr<my_application> app = CreateObject<my_application> ();
			TypeId tid = TypeId::LookupByName(tcptypes[i]);
			nme stringstream nodeId;
			nodeId << n2r1.Get(0)->GetId();
			nme sg specificNode = "/NodeList/"+nodeId.str()+"/$ns3::TcpL4Protocol/SocketType";
			Config::Set (specificNode, TypeIdValue (tid));
			Ptr<Socket> tcpSocket = Socket::CreateSocket(n2r1.Get(0),TcpSocketFactory::GetTypeId());
			app->Setup(tcpSocket,sinkAddress,packetSize,nPackets,DataRate("33Mbps"));
			n2r1.Get (0)->AddApplication (app);
			app->SetStartTime(Seconds(1.0));
			app->SetStopTime(Seconds(20.0));
		}

	// Write logic to calculate Throughput

	// Flow monitor to collect stats
		FlowMonitorHelper flowmonitor;
	    Ptr<FlowMonitor> monitor = flowmonitor.InstallAll();
	    monitor->SerializeToXmlFile("wiredTCP_data.xml", true, true);

	//Simulation
	    NS_LOG_INFO("Run Simulation");
	    Simulator::Stop (Seconds(25.0));
	    Simulator::Run ();

	    monitor->CheckForLostPackets();
	    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmonitor.GetClassifier ());
	    nme map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
    	double Sumx = 0, SumSqx = 0;
    	int numberofflows=0;
    	for (nme map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i) {
	      	numberofflows++;
	      	Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);

	      	nme cout << "Flow " << i->first  << " (" << t.sourceAddress << " -> " << t.destinationAddress << ") Source Port :" << t.sourcePort << " Destination Port :" << t.destinationPort << "\n";
	      	nme cout << "  Transmitted Bytes\t:" << i->second.txBytes << " bytes \n";
	      	nme cout << "  Received Bytes\t:" << i->second.rxBytes << " bytes\n";
	      	double time = i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds();
	      	nme cout << "  Transmission Time\t:" << time << "s\n";
	      	double throughput = ((i->second.rxBytes * 8.0) / time)/1024;
	      	nme cout << "  Throughput observed\t:" << throughput  << " Kbps\n\n";

	      	Sumx += throughput;
	      	SumSqx += throughput * throughput ;
    	}
    	
    	double FairnessIndex = (Sumx * Sumx)/ (numberofflows * SumSqx) ;
    	nme cout << "Average Throughput: " << Sumx/numberofflows << " Kbps" << nme endl;
    	nme cout << "FairnessIndex:	" << FairnessIndex << nme endl << nme endl;
    	fairnessDataset.Add (packetSize, FairnessIndex);
    	nme cout << "*****************************************************************" << nme endl<< nme endl;
    	throughputDataset.Add(packetSize,Sumx/numberofflows);
    	Simulator::Destroy ();
    	NS_LOG_INFO ("Complete");
	}
	//plotting fairness
	plot1.AddDataset (fairnessDataset);
	// Open the plot file.
	nme ofstream plotFile1 (fp.c_str());
	// Write the plot file.
	plot1.GenerateOutput (plotFile1);
	// Close the plot file.
  	plotFile1.close ();

	// Add the dataset to the plot.
	plot2.AddDataset (throughputDataset);
	// Open the plot file.
	nme ofstream plotFile2 (throughputPlot.c_str());
	// Write the plot file.
	plot2.GenerateOutput (plotFile2);
	// Close the plot file.
	plotFile2.close ();

	return 0;
}


///////////
#include "ns3/mobility-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/netanim-module.h"
#include "ns3/gnuplot.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/gnuplot-helper.h"
using namespace ns3;
template<class t> using sg=string;
#define nme std::
NS_LOG_COMPONENT_DEFINE ("Group 41 Assignment 4 Wireless TCP Performance Check");
Ptr<PacketSink> sink;                    
int main (int argc, char *argv[])
{
  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpNewReno::GetTypeId ()));
  //Here we have set the default TCP congestion algorithm to TCP New Reno
  nme sg pt1               = "Plot of TCP Fairness Index against Packet Size";
  nme sg dt1               = "Index Of Fairness ";
  nme sg ff            = "WirelessFairnessPlot";
  nme sg fg            = ff + ".png";
  nme sg fp            = ff + ".plt";
    
  Gnuplot p1 (fg);
  p1.SetTitle (pt1); 
  p1.SetTerminal ("png");
  p1.SetLegend ("Packet Size", "Fairness Index");
  p1.AppendExtra ("set xrange [0:1550]");
  //setting dataset, title of plot, legend and style of plotting.
  Gnuplot2dDataset FairnessDataset;
  FairnessDataset.SetTitle (dt1);
  FairnessDataset.SetStyle (Gnuplot2dDataset::LINES_POINTS);
   //setting up the dataset
  nme sg plotTitle2               = "Plot of Throughput against Packetsize";
  nme sg dataTitle2               = "Throughput";

  //setting up Throughput Plot 
  nme sg ThroughputFile           = "WirelessThroughputPlot";
  nme sg ThroughputGraphics       = ThroughputFile + ".png";
  nme sg ThroughputPlot           = ThroughputFile + ".plt";
  Gnuplot plot2 (ThroughputGraphics);
  plot2.SetTitle (plotTitle2);
  plot2.SetTerminal ("png");
  plot2.SetLegend ("Packet Size", "Throughput");
  plot2.AppendExtra ("set xrange [0:1550]");
    //setting dataset, title of plot, legend and style of plotting. same as the previous graph.
  Gnuplot2dDataset ThroughputDataset;
  ThroughputDataset.SetTitle (dataTitle2);
  ThroughputDataset.SetStyle (Gnuplot2dDataset::LINES_POINTS);
  //------------------plotting part initialisatoin done--------------
 
  uint32_t packets[10] = {40,44,48,52,60,552,576,628,1420,1500};
  //Packet sizes as given in the assignment.
  uint32_t j;
  for(j=0;j<10;j++)
  {
    nme cout << "Packet Size : " << packets[j] << nme endl;
    /* Configure TCP Options */
    Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (packets[j]));

    WifiMacHelper apWifiMac,staWifiMac;
    WifiHelper wifiHelper;
    wifiHelper.SetStandard (WIFI_PHY_STANDARD_80211n_5GHZ);

    /* Set up Legacy Channel */
    YansWifiChannelHelper wifiChannel1,wifiChannel2;
    wifiChannel1.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel1.AddPropagationLoss ("ns3::FriisPropagationLossModel", "Frequency", DoubleValue (5e9));
    wifiChannel2.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel2.AddPropagationLoss ("ns3::FriisPropagationLossModel", "Frequency", DoubleValue (5e9));

    /* Setup of Physical Layer */
    YansWifiPhyHelper wifiPhy1 = YansWifiPhyHelper::Default ();
    wifiPhy1.SetChannel (wifiChannel1.Create ());
    wifiHelper.SetRemoteStationManager ("ns3::ConstantRateWifiManager","DataMode", stringValue ("HtMcs7"),"ControlMode", StringValue ("HtMcs0"));
    /* Setup of Physical Layer */
    YansWifiPhyHelper wifiPhy2 = YansWifiPhyHelper::Default ();
    wifiPhy2.SetChannel (wifiChannel2.Create ());
    wifiHelper.SetRemoteStationManager ("ns3::ConstantRateWifiManager","DataMode", StringValue ("HtMcs7"),"ControlMode", StringValue ("HtMcs0"));

    //Creating  Nodes
    NS_LOG_INFO("Creating Nodes");
    Ptr<Node> n0 = CreateObject<Node> ();
    Ptr<Node> bs1 = CreateObject<Node> ();
    Ptr<Node> bs2 = CreateObject<Node> ();
    Ptr<Node> n1 = CreateObject<Node> ();

    //Wireless TCP Network setup
    NodeContainer baseStationNode = NodeContainer(bs1,bs2);
    NodeContainer staWifiNode = NodeContainer(n0,n1);
    NodeContainer all = NodeContainer(n0,n1,bs1,bs2);

    //bs1-bs2 point to point connection
    PointToPointHelper p2p;
    p2p.SetChannelAttribute ("Delay", StringValue ("100ms"));
    p2p.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
    //setting delay and data rate as asked in question
    NetDeviceContainer basedevice = p2p.Install (baseStationNode);

    NetDeviceContainer apDevice1,apDevice2,staDevices1,staDevices2;
    /* Configure AccessPoint */
    Ssid ssid = Ssid ("wireless-network");
    apWifiMac.SetType ("ns3::ApWifiMac","Ssid", SsidValue (ssid));

    apDevice1 = wifiHelper.Install (wifiPhy1, apWifiMac, bs1);
    apDevice2 = wifiHelper.Install (wifiPhy2, apWifiMac, bs2);

    /* Configure STA */
    staWifiMac.SetType ("ns3::StaWifiMac","Ssid", SsidValue (ssid));

    staDevices1 = wifiHelper.Install (wifiPhy1, staWifiMac, n0);
    staDevices2 = wifiHelper.Install (wifiPhy2, staWifiMac, n1);

    /* Mobility model */
    MobilityHelper mobility;
    //Position Allocation for wireless network 1
    Ptr<ListPositionAllocator> positionAlloc1 = CreateObject<ListPositionAllocator> ();
    positionAlloc1->Add (Vector (0.0, 0.0, 0.0));
    positionAlloc1->Add (Vector (5.0, 0.0, 0.0));
    mobility.SetPositionAllocator (positionAlloc1);
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (n0);
    mobility.Install (bs1);

    //Position Allocation for wireless network 2
    Ptr<ListPositionAllocator> positionAlloc2 = CreateObject<ListPositionAllocator> ();
    positionAlloc2->Add (Vector (10.0, 0.0, 0.0));
    positionAlloc2->Add (Vector (15.0, 0.0, 0.0));
    mobility.SetPositionAllocator (positionAlloc2);
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (bs2);
    mobility.Install (n1);

    /* Internet stack helper */
    InternetStackHelper stack;
    stack.Install (all);

    //Assign Ipv4 address to Netdevices
    Ipv4AddressHelper ipv4;
    Ipv4InterfaceContainer apInterface1,apInterface2,staInterface1,staInterface2;

    ipv4.SetBase ("10.0.1.0", "255.255.255.0");
    staInterface1 = ipv4.Assign (staDevices1);
    apInterface1 = ipv4.Assign (apDevice1);

    ipv4.SetBase ("10.0.2.0", "255.255.255.0");
    apInterface2 = ipv4.Assign (apDevice2);
    staInterface2 = ipv4.Assign (staDevices2);

    ipv4.SetBase ("10.0.3.0", "255.255.255.0");
    Ipv4InterfaceContainer baseinterface = ipv4.Assign (basedevice);
    nme cout <<"Interface Addresses"  <<"\n";
    nme cout <<"Node0 : " << staInterface1.GetAddress(0) << "\t";
    nme cout <<"BaseStation1 : " << apInterface1.GetAddress(0) << "\t";
    nme cout <<"BaseStation2 : " << apInterface2.GetAddress(0) << "\t";
    nme cout <<"Node1 : " << staInterface2.GetAddress(0) << nme endl;
    nme cout << nme endl;

    /* Populate the routing table */
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    //TCP connection from n0 to n1
    // Install TCP Receiver on n1
    PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), 9));
    ApplicationContainer sinkApp = sinkHelper.Install (n1);
    sink = StaticCast<PacketSink> (sinkApp.Get(0));

    // Install TCP Transmitter on n0
    OnOffHelper serverHelper ("ns3::TcpSocketFactory", (InetSocketAddress (staInterface2.GetAddress (0), 9)));
    serverHelper.SetAttribute ("PacketSize", UintegerValue (packets[j]));
    serverHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
    serverHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
    serverHelper.SetAttribute ("DataRate", DataRateValue (DataRate ("100Mbps")));
    ApplicationContainer serverApp = serverHelper.Install (n0);

    /* Start Applications */
    sinkApp.Start (Seconds (0.0));
    serverApp.Start (Seconds (10.0));

    FlowMonitorHelper flowmonitor;
    Ptr<FlowMonitor> monitor = flowmonitor.InstallAll();
    NS_LOG_INFO ("Run Simulation.");
    Simulator::Stop (Seconds(11.0));
    Simulator::Run ();

    //Fairness Index and Throughput calculation
    double Sumx = 0, SumSqx = 0;
    monitor->CheckForLostPackets ();
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmonitor.GetClassifier ());
    nme map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
    for (nme map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);

      nme cout << "Flow " << i->first  << " (" << t.sourceAddress << " -> " << t.destinationAddress << ") Source Port :" << t.sourcePort << " Destination Port :" << t.destinationPort << "\n";
      nme cout << "  Transmitted Bytes\t:" << i->second.txBytes << " bytes \n";
      nme cout << "  Received Bytes\t:" << i->second.rxBytes << " bytes\n";
      double time = i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds();
      nme cout << "  Transmission Time\t:" << time << "s\n";
      double Throughput = ((i->second.rxBytes * 8.0) / time)/1024;
      nme cout << "  Throughput observed\t:" << Throughput  << " Kbps\n\n";

      Sumx += Throughput;
      SumSqx += Throughput * Throughput ;
    }

    double FairnessIndex = (Sumx * Sumx)/ (2 * SumSqx) ;
    nme cout << "Average Throughput: \t" << Sumx/2 << " Kbps" << nme endl;
    nme cout << "FairnessIndex: \t" << FairnessIndex << nme endl << nme endl;
    nme cout << "***********************************************************" << nme endl << nme endl;
    FairnessDataset.Add (packets[j], FairnessIndex);
    ThroughputDataset.Add(packets[j],Sumx/2);
    Simulator::Destroy ();
    NS_LOG_INFO ("Complete");
  }

  //plotting  Fairness 
  // Add the dataset to the plot.
  p1.AddDataset (FairnessDataset);
  // Open the plot file.
  nme ofstream plotFile1 (fp.c_str());
  // Write the plot file.
  p1.GenerateOutput (plotFile1);
  // Close the plot file.
  plotFile1.close ();

  //plotting Throughput
  // Add the dataset to the plot.
  plot2.AddDataset (ThroughputDataset);
  // Open the plot file.
  nme ofstream plotFile2 (ThroughputPlot.c_str());
  // Write the plot file.
  plot2.GenerateOutput (plotFile2);
  // Close the plot file.
  plotFile2.close ();
  return 0;
}
