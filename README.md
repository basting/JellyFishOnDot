JellyFish On Dot (Distributed OpenFlow Testbed)
==============

Source code and other files used for JellyFish experiments conducted on DOT
=============================================================================
To generate the configuration file, use topology_V1.cpp
=======================================================
1) compile this file, in command line, type:
    g++ -o work topology_V1.cpp
2) Run the file with parameters <nHosts> <nSwitches> <nPorts>
    Eg: to generate a topology with 16 hosts, 20 switches, and 4 free ports on each switch, in command line, type:
    ./work 16 20 4
    The result will be saved in JellyFish.txt
3) IMPORTANT:
    using cut-paste, put the section [LogicalTopology] between [PhysicalTopologyDelay] and [Images]
    save the file.

=============================================================================
topology_V2.cpp also takes of calculating the path lengths
=============================================================================
1) compile this file, in command line, type:
    g++ -o work topology_V2.cpp

2) Run the file with parameters <nHosts> <nSwitches> <nPorts>    
./work 128 90 8

Sample output:
============================================================================
consecFails: 13
timer 16384
0 128
2 76
3 1126
4 5220
5 9120
6 714

Connectivity test : passed
*****************************************
!!Now cut and paste [LogicalTopology] part in-between [PhysicalTopologyDelay] and [Images]

*****************************************
============================================================================
