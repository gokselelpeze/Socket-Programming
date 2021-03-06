# Socket-Programming
Operating Systems Homework

# Assignment#3: DEU Signal v1.0

# Goal
In this assignment, you will implement server side and client side socket programming.

# Implementation Details & Requirements
Nowadays, instant messaging applications are used very often by end users. WhatsApp, the
most popular of these applications, recently announced that it will make some changes in the
security policy it will follow in our country. With this development, such applications
immediately have been discussed by people and the search for a more secure application has
started.<br>

With this assignment, you will start to develop an instant messaging application. You will
implement the first phase for this application. In the first phase, people can send non-secure
messages to others using the messaging server. They can be sent and received using JSON
format that contains from, to and message key-fields. In this system, we must have a
messaging server and we may have more than a client.<br>

The server must be running up always for the system to work. It executes commands which
are taken by clients and controls and manages the system. The server will be hosted on
localhost and its port is 3205. When the client connects successfully to the server, it
will enter its private phone_number (username) initially. Then it is defined in the
server system.<br>

● username must be similar to phone number like [08500232000] <br>

The commands are described in below can be used in the system:
● -gcreate group_name: Creates a new specified group. The groups have been protected with non-encrypted passwords. The system will ask to define a password.<br>
● -join username/group_name: Enter to the specified username or group name. If the group is private, the client must know the password for entering.<br>
● -exit group_name: Quit from the group that you are in.<br>
● -send message_body: Send a JSON-formatted message to the group that you are in.<br>
● -whoami Shows your own username (phone_number) information.<br>
● -exit Exit the program.<br>
