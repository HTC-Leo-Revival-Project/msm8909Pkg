# What is it?
A very simple implementation of tetris for UEFI platform.

For a rationale, see [the essay (in polish)](Esej.pdf)

For a deeper insight into implementation, see [the report (also in polish)](Sprawozdanie.pdf)

# Building and launching the application
Use the Visual Studio solution located under *NT32/NT32.sln*, note that path to the project mustn't contain spaces, otherwise the compilation will fail.
After the initial build *EDK_II/Conf/target.txt* will be created, MAX_CONCURRENT_THREAD_NUMBER value should be adjusted according to its description to optimize the building process.

Project may be launched directly from Visual Studio (f5 by default). One console window and two EFI shell windows should appear upon launch, then:
* select one of the windows with EFI shell
* select filesystem by typing `f8:`
* execute `tetris.efi`

# Credits
The implementation is largely based on https://github.com/swmicro/Tetris