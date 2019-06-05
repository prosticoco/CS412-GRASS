All exploits that rely on hijaking the flow are dependent on the actual address of the function hijack_flow, so if the exploits do not work it is most likely that the addresses are different.
If this is the case, gdb can be used to find the address of the function ( by breaking main, running and printing hijack_flow). These exploits were designed, written and tested to work with a Kali 64-bit OS.
For any questions, you can contact us at adrien.prost@epfl.ch, rodrigo.soaresgranja@epfl.ch

Exploit 1 : Command Injection : 
Throughout the code we check for invalid characters in the commands using a dictionnary : "&|;$><\!`".
We created a function that does this and takes the size of the dictionnary as an argument. The vulnerability is in the command cd where:
int check_size = DICT_SIZE;
if(!checkInvalidChars(curr_co->curr_args[0],--check_size)) ...
You can see that we have set the -- operator before check_size thus decrementing it, which means that the last char in the dictionnary will not be checked. It is now possible to use the back ticks to take precedence over the cd command (cd `xcalc`) thus spawning a calculator.


Exploit 4 : Hijack-flow with a buffer overflow : 
In the command grep we must pass a pattern as an argument. We set the max argument value to 128 for every command argument and we do the same here. We need to check for the validity of the pattern (invalid chars, command injection, etc) so we created a new function to do this and added the vulnerability there. In this new function we initialize a buffer of size 64 which is the max size of the pattern as defined by us. So the vulnerability is that we copy the whole 128 byte long string into this 64 byte buffer which can then be overflown. We use this to overflow the buffer and overwrite the RIP address with the hijack_flow function address, thus hijacking the flow :)


Exploit 5 : Achieving authentication with buffer overflow :
We use a structure for all data related to a single client-server connection. In this structure we use a char buffer to store the temporary username given to us by the login function. In the structure this buffer is directly followed by a bool variable which is used to verify the authentication of the client connection.
In the login function we check if the username is too long and return an error if it is. Nevertheless, the vulnerability is that the user is still able to achieve an overflow of 1 byte, thus being capable to directly modify the value of the authentication bool variable to a value different than 0 (setting it true) and thus achieve higher privilege. 

