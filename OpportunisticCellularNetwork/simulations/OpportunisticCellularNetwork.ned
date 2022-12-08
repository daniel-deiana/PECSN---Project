package opportunisticcellularnetwork.simulations;

import opportunisticcellularnetwork.Antenna;
import opportunisticcellularnetwork.Cellular;
import opportunisticcellularnetwork.Source;


network OpportunisticCellularNetwork
{
    parameters:
        int population;
        @display("bgb=1146,916,#DFDFDF,,1");
    submodules:
        antenna: Antenna {
            @display("i=device/antennatower,,0;is=vl;p=538,292");
        }
        cellular[population]: Cellular {
            id = index;
            @display("i=device/cellphone;is=s;p=880,485");
        }
        source[population]: Source {
            id = index;
            @display("i=device/server2;is=n;p=216,482");
        }
    connections:
        for i=0..(population-1) {
            cellular[i].out --> antenna.inCellular++;
            source[i].out --> antenna.inSource++;
            antenna.out++ --> cellular[i].in;
        }
}
