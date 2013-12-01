### Appmem Architecture Notes

In implementing the basic appmemlib I ran across a number of architectural considerations.  As the architecture is a work in progress, this just a free-form ramble.

The idea of an Appmem Device is giving Applications a large amount of RAM through an API.   How to do this?  

### Sync vs Async

To an Application, System RAM is Synchronous.  Dig into hardware, things are not that simple.   Memory Reads/Writes first go to CPU cache, then out to System RAM if there is a cache miss.  If the CPU encounters a cache miss, then it may perform some CPU tricks like out-of-order execution to “hide” this stall while the memory is fetched.    

For PCIe, we could use MMIO (Memory Mapped I/O).   This would map the Device’s internal memory to System Addresses.  Then applications could use this memory directly.   However, this presents a number of issues.    Caching MMIO is complex and difficult.   If we’re considering using an Appmem Device, then we’re probably considering very large (multi-GB) amounts of RAM to begin with, so caching is nice but probably not that useful.    PCIe latency is in the 300ns range, so the stalls we have with System RAM are multiplied by 10x.   It’s doubtful that CPU tricks can deal with that much of stall effectively.

On the other side of the coin, we have Asynchronous I/O.   This is any file or network based I/O.   A request is made, the process goes to sleep and wakes up when the data is ready (or gets a callback).   To the programmer, this may not be apparent :  fread() will put the process to sleep and wake up when it’s done, so unless the application is multi-threaded, it might not take advantage of this.     

So that’s the crux of the problem – Seems like we have two choices. 

   1.	Make Async operations look Synchronous to the application.   
   2.	Interface with synchronous data structures asynchronously. 
  
Of the two choices, I think #1 is the way to go.    #2 might be an option to have, but I believe the basic interface should remain synchronous.      One of the fundamental features of appmemlib is that it can use the same data structures and algorithms using either System RAM (virtd) or Device Memory.  If we make System RAM access asynchronous, we negate much of the speed of System RAM by having the overhead of context switches or callbacks, with no benefit for the application.

Disaggregating Application Memory from the CPU is an architectural change.    Forcing synchronous operation in the API will require applications to make changes to their applications that can take advantage of the asynchronous and offload properties of the devices.  For the moment, we are focused on the mega data-center – where programmers are willing to make software architectures changes for scale-out, cost and performance advantages.    Not going spend too much time thinking about legacy software at this point. 

### C++ Containers / Appmem Devices do not return references to stored objects

This is very much like file or block storage.   Regardless of the amType (Associative Array, List, Tree, etc..) the object stored is just binary data.    Functions such as Sort() or Find() may take data types (i.e. string, word-width, float type)  offsets and into the object, but otherwise, the device will return a copy of the object. Changes to elements of an object just change the local copy.     If they application needs to change to data, it needs to write the object again, or perform a TBD modify operation.   

For the C++ API, I would like to keep some of the semantics of the STL Containers.   But I don’t want to confuse the programmer either --- it’s not the same.   I’d like also to provide optimizations for devices (for example, on an insertion, you might provide a callback on error but otherwise the operation to the device would appear completely synchronous to the application.)

I also don’t know if this is helpful for other language wrappers or not.   If I keep C++ Container semantics, does that help or hurt when exporting this to Java, JS or PHP? 

I’m way rusty on C++ anyway, so won’t do much work in this area until I get some feedback from application developers and try some real world use cases.

### Generic Driver Interface

For the user mode emulation (virtd) we can just pass pointers and key/data length directly to the function module.    When the length of either key or data is fixed (I refer to this as aligned in the code), we can just pass one or two pointers.

This works for packet mode as well.  The same accessor functions take pointers to offsets in the packet.   I’ve used this method in both the network packet interface as well as the MMIO interface to kernel emulation.    This simple packet mode incurs 8 bytes of header overhead (should work up to data packets sizes of 16K, otherwise, we’ll need larger a jumbo header, but don’t have support for that yet.)

In kernel mode IOCTL mode, I’ve used a similar packet model, with the user mode pointers being sent in the packet.   This works find but requires user-to-kernel buffer copies before the data is processed, overhead that likely unacceptable in real world mode.  Plus, we want a native interface and not an ioctl interface.

The ideal interface would pass a “packet” directly to kernel, as well as a source/target data buffer, with a callback interface for the completion back to user mode.    None of the native Linux device classes seem suitable to that.    Char/block pass an integer (offset/LBA) as key into the data in the read/write methods – we need free form “keys” into data.    Network drivers are packet based but one way – we need a method with a response that may be asynchrounous (callback).

The SCSI model fits the requirement.   In SCSI, the CDB is 6-16+ bytes of “command” as well as a user data buffer that can be SGL for DMA.   There is unnecessary overhead going through the SCSI stack – and given all my kernel experience prior to this project is in the SCSI driver, seems like too easy of an answer.

I think it would be relatively easy to design a new interface for the kernel that is both lightweight and can fully abstract all the possible device operations (MMIO, DMA, RDMA, interrupts, network packets).    However, users aren’t going to want to modify the kernel, at least until the technology is proven.

This is a good discussion topic.   The first few “real” hardware devices will probably influence this decision point.

### Object Storage

Conceptually, Appmem Devices + Persistence is similar to Object Storage (including SCSI Object Storage (OSD)).   We don’t want to impose a standard specification what Appmem Devices can do – that power needs to remain in the hands of the application programmer.  However, it may be worth taking a look at past attempts in this area to leverage terminology and interface solutions.
