-> To run the kernel module use the commands as follows
1) make 
2) sudo insmod ./sysfs_example.ko -> to initialize the module
		Then this kernel module will be initialized
		
To use :-
========
Writing :-
	echo "Data" > /sys/sasken/training
Reading :-
	cat /sys/sasken/training
	
then we get the output what we written in that file previously by default it is NULL

3) sudo rmmod sysfs_example -> to remove the module

Thanks for going through the code...!
