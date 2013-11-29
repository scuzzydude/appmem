<b>Appmem devices – Disaggregating RAM in the data center.</b>

I was thinking about disaggregation in the data center a few years from now; with all the server components connected through a common fast interconnect such as Intel’s SiPho and the cool possibilities.  And then come to the realization that two of the most expensive components – CPUs and System Memory, can’t be easily disaggregated.
### Latency
The reason why is latency.   While interconnect bandwidth will continue to rise, and optical-on-chip will make high bandwidth systems easier to design, there are several factors that prevent memory latency from improving at the same rate.   Some of these are business, some are technical, but the fundamental reasons are physical.  Distance == latency on a nanosecond scale.   When we’re talking about flash with microsecond latency, adding tens of nanoseconds for the sake of system disaggregation is no big deal.   When we’re talking about System RAM (~15-20 ns of theoretical latency) every nanosecond counts.   On cache misses for code or kernel data, a multi-core CPU can stall completely.   This results in a cascade of delays. 
### Ram Sled
While all RAM in the system is treated equally, to applications, it may not be.    Facebook has realized this with their “RAM Sled” concept.   This basically puts a large amount of RAM on a card that might be connected via PCIe or other interface, and having a key-value store.   Ideally, this could be pooled and shared between systems.    To applications programmers, this key-value store is similar to a common data structure called an Associative Array.
### Abstracting RAM function 
Appmem Devices take this a step further.   Rather than just key-value stores, Appmem Devices will be capable of storing elements in more complex data structures.   Arrays, Lists, Stacks, Queues, Trees.   The idea is to abstract any kind of data structure that programmers may need large chunks of RAM for.
### Appmem Device Characteristics
1.	Appmem Memory may or may not be persistent.
2.	Appmem Memory may or may not be shared.
3.	Interconnect is irrelevant.
4.	They are categorized by latency --- at the application.   Leveraging CPU cache convention: 
•	A0 has 0-10ns latency 
•	A1 100ns 
•	A2 1us 
•	A3 100us.
5.	They share a common API and algorithm library – the same code can run on System RAM.
### Offload
Appmem Devices have another benefit.  Complex routines that normally run on the CPU can be offloaded to the Appmem devices.   Inserting into an Appmem Device Tree should have the same approximate CPU cost as a memory write.   A sort of a 4GB Array that would take seconds of CPU time can be offloaded and the CPU can use those cycles for something useful.
### Interprocess and intersystem sharing
For complex multi system applications, the possibilities are even more profound.   Rather than saving a 10GB Tree data structure to a file, and then handing the file to another system for further processing, the Application can just hand off the handle to the Tree, and the 2nd system can immediately begin processing.  No file or disk I/O.
### Hardware design driven by programmers
I’ve been pretty vague about what these devices look like – intentionally.     In the old world of hardware design, you’d have to be very specific.   Vendors would fight for years over a standard specification, and then when devices were released, additional years of engineering effort would be required to make them work with each other.    In the new world (OCP and “software defined hardware”), I think vagueness is the way to make this work, especially in the data center.
1.	Appmem Memory may or may not be persistent
The Device may be a server with a file system, or an embedded device with flash, or the client may be able to pull the data and save it.   It’s RAM – persistence may not be important.  Persistence is value add, but not core to the device’s function.
2.	 Appmem Devices may or may not be shared.   The model we’re using is data structures application programmers use.   In the current model, these aren’t usually shared with other systems directly.   This is a value add feature.
3.	Interconnect is irrelevant.   What is important is how the Device interacts with the client library.  Supporting multiple interconnects (Ethernet/PCIe/ others) is important, and design at the client library needs to consider the differences, but it should be abstracted so that any interconnect can work.   If you want to design a 1TB A10 device connect by 14.4 kbs Modem, you should be able to do it.  (My first target device, a Raspberry Pi connected via UDP over 100BASE-TX isn’t much faster, but it works – sort of.)
4.	Categorized by latency – measured at the application.  Let us say our device is a full-fledged appliance, with connections to the client through Infiniband, Ethernet,  RoCE, as well as through PCIe Fabric.   One Ethernet client may be 20 switch hops away, and measures a latency of A3.   Clients connected through PCIe may measure A1.    The Client may have hundreds of devices with varying capacity and latency to choose from, it can pick the most appropriate for its needs.
5.	They share a common API library.   This is the most important factor to make this effort successful.   Application programmers can take the library, without a device, and run the algorithms using System RAM.   When a device is available, the work is done, and programmers can measure System RAM performance against Appmem Device performance.   
•	Programmers should be able to add new data structures to the library with little effort.  The management of communication with the devices should be abstracted from the data algorithms.
•	Programmers that develop new data structures should be able to hand over their code to Device vendors for direct customization.
•	Alternately, Vendors may have SDK where customers can integrate their custom data structures.   If the devices are Linux based, this may be as simple as compiling the library on the device itself.  

The key to making this work is the API.   I’ve made a first attempt at this, with my project appmemlib.   It’s a simple C/C++ interface, test program, virtual client, kernel virtual client driver, and target device.   For now, it supports Flat Memory, Static Arrays, and Associative Arrays.  I plan to add a few more basic data structures.  

I’m a firmware/driver guy.  I’m not an applications programmer, and certainly not a big-data programmer.  I certainly never needed more than a few MB of RAM for anything I’ve done in user space.   So figuring out HOW to use this conceptually is the next step.   I’m looking for help from the community for this.      I’m looking for test cases and use cases that might help prove out and demo the idea.    The basic use case is applications that use large chunks of RAM temporarily, and perform operations (sorting/searching) on them that could be offloaded. 

Just from this early work, I’ve uncovered a number of Architectural considerations.   So far nothing that is a fundamental roadblock.    RAM disaggregation requires rethinking how we use it in the system – The physical and business considerations of NOT disaggregating RAM are pretty fundamental.
