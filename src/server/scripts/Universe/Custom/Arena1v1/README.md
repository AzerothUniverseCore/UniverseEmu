# npc_arena1v1
1v1 for SyphrenaCore 3.3.5 - Updated for the latest TC (10-March-2019)  
    
Original Mod written by Quadral can be found here: https://github.com/Quadral/335source/tree/master/src/server/scripts/Custom/1V1

**UPDATE:** Tested in-game and seems to be working for me. Reload config command is working as well.  
  
## How to install

**1.** Navigate to your **SyphrenaCore\src\server\scripts\Custom** and make a new folder called **Arena1v1**.
Copy **npc_arena1v1.cpp** and **npc_arena1v1.h** to your **Arena1v1** folder you just created. (this is optional but I recommend it just to keep things organized)

***

**2.** Add 1v1 arena script to SyphrenaCore\src\server\scripts\Custom\ **custom_script_loader.cpp**

![alt tag](https://i.imgur.com/zHKZ2nC.png)

***

**3.** Use CMake (configure and generate)

***

**4.** Open **SyphrenaCore.sln** and Build the solution. Click on **Build** and then **Build Solution** or press (Ctrl+Shift+B)

![alt_tag](https://i.imgur.com/fQwUCML.png)

***

**5.** Add the following at the end of your **worldserver.conf** file  

```cpp
###################################################################################################
#  1v1
#        Arena.1v1.Enable
#	     Description: Defines is the world chat enabled or disabled, default : Enabled
#		 Usage: 0 = Disabled, 1 = Enabled

Arena.1v1.Enable = 1

#
#
#  1v1
#        Arena.1v1.Enable
#	     Description: Gives value cost; 400000 is 40g
#		 Usage: 0-400000 // 400k is 40g

Arena.1v1.Costs = 400000

#
#  1v1
#        Arena.1v1.MinLevel
#	     Description: Change player level requirement to join 1v1 arena
#		 Usage: 1-255

Arena.1v1.MinLevel = 80

#
###################################################################################################
```

***

**6.** Execute **create_npc_world_database.sql** to your database.
 

