<h1>UNIVERSAL TAGGING SYSTEM</h1>
<h2>Things to do</h2>

- [x] Create the Commandline Universal Tagging System (CUTS)

- [x] Test for windows compatibility

- [x] Create a makefile for this program

- [x] Create the Graphical Universal Tagging System (GUTS)

<h2>Welcome to UTS</h2>
UTS is a program designed to make the navigation of file systems and finding particular files easier by using simple lists combined with tags for your conveniance. Create lists of files or directories you want to keep track of in individual lists which then can be further seperated for up to 10 tags. Enjoy both <strong>CUTS</strong> (Command line Universal Tagging System) and <strong>GUTS</strong> (Graphical Universal Tagging System) for your needs.
<h2>How to use</h2>
<ol>
    <li>Git clone the directory for your desired version of the UTS. For this example, we will use CUTS, which can be done by using this command:</li>

        >    git clone https://github.com/cheekibreeki2401/UTS/tree/master/CUTS
    
 <li>Use the make file included in that directory with the target of the system (linux, win64, win32):</li>

        > make TARGET=linux

  <li>Run the program of your choice by terminal using:

        >    ./CUTS
   
 </li>
    <li>Follow prompts to start building your lists, simple!</li>
<strong>NOTE:</strong> GUTS requires the use of the IUP graphical library to build.
</ol>
