BEFORE GOING TO RUN, PLEASE GO THROUGH THE PPT IN THE FOLDER


-> To run the kernel module use the commands as follows
1) make 
2) sudo insmod ./panic_example.ko -> to initialize the module
		Then this kernel module will be initialized
		
To use :-
========


Writing :-
TO CREATE A PANIC IN THE KERNEL
	echo "panic" > /sys/sasken/training
	
	
	
Reading if needed:-
	cat /sys/sasken/training
	
then we get the output what we written in that file previously by default it is NULL

3) sudo rmmod panic_example -> to remove the module

Thanks for going through the code...!
