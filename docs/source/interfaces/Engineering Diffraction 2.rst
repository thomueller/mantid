.. _Engineering_Diffraction_2-ref:

Engineering Diffraction 2
=========================

.. contents:: Table of Contents
    :local:

Interface Overview
------------------

This custom interface will integrate several tasks related to engineering
diffraction. In its current state it provides functionality for creating
new calibration files. This interface is under active development.

General Options
^^^^^^^^^^^^^^^
RB Number
    The reference number for the output paths (usually an experiment reference
    number at ISIS). Leaving this field empty will result in no user directories
    being created, and only the general directory will be used for file storage.

Instrument
    Select the instrument (ENGINX or IMAT). Currently only ENGINX is fully
    supported.

?
    Show this documentation page.

Close
    Close the interface.

Red Stars
    Red stars next to browse boxes and other fields indicate that the file
    could not be found. Hover over the star to see more information.

Calibration
-----------

This tab currently provides a graphical interface to create new calibrations
and visualise them. It also allows for the loading of GSAS parameter files created
by the calibration process to load a previously created calibration into the interface.

When loading an existing calibration, the fields for creating a new calibration will be
automatically filled, allowing the recreation of the workspaces and plots generated by
creating a new calibration.

The "Plot Output" check-box will plot vanadium curves and ceria peaks for new calibrations.
Four plots will be generated (for ENGINX), one of each plot for each of the detector banks.

Creating a new calibration file generates 3 GSAS instrument parameter files,
one covering all banks and separate ones for each individual bank. All 3 files are written
to the same directory:

`Engineering_Mantid/Calibration/`

If an RB number has been specified the files will also be saved to a user directory
in the base directory:

`Engineering_Mantid/User/RBNumber/Calibration/`

Currently, the `Engineering_Mantid` directory is created in the current user's home directory.

Parameters
^^^^^^^^^^

Vanadium Number
    The run number or file path used to correct the calibration and experiment runs.

Calibration Sample Number
    The run number for the calibration sample run (such as ceria) used to calibrate
    experiment runs.

Path
    The path to the GSAS parameter file to be loaded.