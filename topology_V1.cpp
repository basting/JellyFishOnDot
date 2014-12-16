#include <iostream>
#include <fstream>
#include <cstdio>
#include <vector>
#include <math.h>
#include <cstdlib>
#include <time.h>
#include <string>
#include <set>

using namespace std;

void checkConnectivity(int node, vector<bool> &visited, vector<vector<int> > &graph) {
	visited[node] = true;
	for ( int i=0 ; i<visited.size() ; i++ ) {
		if ( graph[node][i] && !visited[i] ) {
			checkConnectivity(i, visited, graph);
		}
	}
}

int main(int argc, char** argv) {
    //int nHosts = 16;		// # of hosts
    //int nSwitches = 20;		// # of switches
    //int nPorts = 4;			// # of free ports per switch
	int nHosts = atoi(argv[1]);
	int nSwitches = atoi(argv[2]);
	int nPorts = atoi(argv[3]);
	cout << "nHosts = " << nHosts << endl;
	cout << "nSwitches = " << nSwitches << endl;
	cout << "nPorts = " << nPorts << endl;
	
	ofstream fout("JellyFish.txt");
	fout << "[EmulationName]\nJellyFish\n\n";
	
	fout << "[PhysicalTopology]\n3\n";
	fout << "manager dm-grp3 10.0.3.1\n";
	fout << "node cn113 10.0.0.13 16 12288 8 eth0 10240\n";
	fout << "node cn117 10.0.0.17 16 12288 8 eth0 10240\n\n";

	fout << "[PhysicalTopologyDelay]\n0\n\n";
		
    // set # of free ports per switch
    vector<int> openPorts;
    for ( int i=0 ; i<nSwitches ; i++ ) {
        openPorts.push_back(nPorts);
    }
    //int downLinks = ceil((double)nHosts / nSwitches);
    //int upLinks = nPorts - downLinks;


	fout << "[Images]\n2\ni1 tc base_tc.qcow2\ni2 ubuntu base_lubuntu_mini.qcow2\n\n";
	fout << "[VirtualMachineProvision]\n#Start marker of this section. Don't remove\nHypervisor=kvm\nLibrary=lib-virt\nNetworkFile=resources/provisioning/libvirt/libvirt_network.xml\n#End marker of this section. Don't remove\n\n";
	


    // distribute hosts onto switches
    fout << "[VirtualMachines]" << endl;
    fout << nHosts << endl;
    if ( nSwitches >= nHosts ) {
        int hostInterval = (double)(nSwitches-1)/(nHosts-1);
        for ( int i=0 ; i<nHosts ; i++ ) {
            fout << "h" << i+1 << " s" << hostInterval*i+1 << " 1 300 64 i1" << endl;
            openPorts[hostInterval*i]--;
        }
    }
    else {
        vector<int> hps;    // hosts per switch
        for ( int i=0 ; i<nSwitches ; i++ ) {
            hps.push_back(nHosts/nSwitches);
        }
        int hostsLeft = nHosts % nSwitches;
        for ( int i=0 ; i<hostsLeft ; i++ ) {
            hps[i]++;
        }
        int hostCount = 0;
        for ( int i=0 ; i<nSwitches ; i++ ) {
            for ( int j=0 ; j<hps[i] ; j++ ) {
                fout << "h" << 1+hostCount++ << " s" << i+1 << " 1 300 64 i1" << endl;
                openPorts[i]--;
            }
        }
    }
    
    fout << "\n[Controllers]\n3\n";
    fout << "c1 192.168.10.106 6633\n";
    fout << "c2 10.254.0.2 6633\n";
    fout << "c3 10.0.3.1 6633\n\n";
    
    fout << "[Switch2Controller]\n";
    fout << nSwitches << endl;
    for ( int i=0 ; i<nSwitches ; i++ ) {
    		fout << "s" << i+1 << " c3\n";
    }
    
    fout << "\n[Credentials]\n";
    fout << "UserName=dotuser\n";



    int switchesLeft = nSwitches;
    int consecFails = 0;

    set<pair<int,int> > links;
    srand(time(NULL));
    while ( switchesLeft > 1 && consecFails < 2*nHosts ) {		//
        int s1 = rand() % nSwitches;
        while ( openPorts[s1] == 0 ) {
           s1 = rand() % nSwitches;
        }

        int s2 = rand() % nSwitches;
        while ( openPorts[s2] == 0 || s1 == s2 ) {
            s2 = rand() % nSwitches;
        }

        if ( links.find(make_pair(s1,s2))!=links.end() ) {
            consecFails++;
        }
        else {
            links.insert(make_pair(s1,s2));
            links.insert(make_pair(s2,s1));
            openPorts[s1]--;
            openPorts[s2]--;

            switchesLeft -= (openPorts[s1]==0) + (openPorts[s2]==0);
        }
    }

	cout << "\n\nconsecFails: " << consecFails << endl;

    if ( switchesLeft>0 ) {
        for ( int i=0 ; i<nSwitches ; i++ ) {
            while ( openPorts[i]>1 ) {
                while ( true ) {
                    // choose a link
                    set<pair<int,int> >::iterator iter = links.begin();
                    int rIndex = rand()%links.size();
                    advance(iter, rIndex);
                    int node1 = iter->first;
                    int node2 = iter->second;
                    // make sure this link doesnt contain i
                    if ( links.find(make_pair(node1,i))!=links.end() ) {
                        continue;
                    }
                    if ( links.find(make_pair(node2,i))!=links.end() ) {
                        continue;
                    }

                    // remove links
                    links.erase( iter );
                    links.erase( links.find( make_pair(node2, node1)));

                    // add new links
                    links.insert( make_pair(node1, node2));
                    links.insert( make_pair(node2, node1));

                    openPorts[i] -= 2;
                    break;
                }
            }
        }
    }

    // output links
    fout << "\n[LogicalTopology]" << endl;
    set< pair<int,int> >::iterator iter = links.begin();

    // first loop : count the # of valid links and print
    int nLinks = 0;
    for ( int i=0 ; i<links.size() ; i++,advance(iter, 1) ) {
        int node1 = iter->first;
        int node2 = iter->second;
        if ( node1 < node2 ) {
            nLinks++;
        }
    }

    fout << nSwitches << " " << nLinks << endl;

	vector< vector<int> > graph(nSwitches, vector<int>(nSwitches,0));
    // second loop : print all the links
    iter = links.begin();
    for ( int i=0 ; i<links.size() ; i++,advance(iter, 1) ) {
        int node1 = iter->first;
        int node2 = iter->second;
        if ( node1 > node2 ) {
            continue;
        }
        else {
            fout << "s" << node1+1 << " s" << node2+1 << " 300 0" << endl;
            graph[node1][node2]=1;
            graph[node2][node1]=1;
        }
    }
    vector<bool> visited(nSwitches, false);
    checkConnectivity(1, visited, graph);
    bool connect = true;
    for ( int i=0 ; i<nSwitches ; i++ ) {
    		if ( visited[i]==false ) {
    			connect = false;
    			cout << "\n\nGraph not connected, please run again. =(" << endl;
    			break;
    		}
    }
    if ( connect ) {
    		cout << "\n\nConnectivity test : passed" << endl;
    		cout << "*****************************************" << endl;
    		cout << "!!Now cut and paste [LogicalTopology] part in-between [PhysicalTopologyDelay] and [Images]\n" << endl;
    		cout << "*****************************************" << endl;
    }
    
    fout << "\n[OtherConfig]\n";
    fout << "OFVersion=1.0\n";
    
	fout.close();
	return 0;
}
