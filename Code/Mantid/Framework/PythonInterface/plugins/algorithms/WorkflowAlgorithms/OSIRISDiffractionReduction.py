#pylint: disable=invalid-name,no-init
from mantid.kernel import *
from mantid.api import *
from mantid.simpleapi import *

import itertools


time_regime_to_d_range = {\
     1.17e4: tuple([ 0.7,  2.5]),\
     2.94e4: tuple([ 2.1,  3.3]),\
     4.71e4: tuple([ 3.1,  4.3]),\
     6.48e4: tuple([ 4.1,  5.3]),\
     8.25e4: tuple([ 5.2,  6.2]),\
    10.02e4: tuple([ 6.2,  7.3]),\
    11.79e4: tuple([ 7.3,  8.3]),\
    13.55e4: tuple([ 8.3,  9.5]),\
    15.32e4: tuple([ 9.4, 10.6]),\
    17.09e4: tuple([10.4, 11.6]),\
    18.86e4: tuple([11.0, 12.5])}\


class DRangeToWsMap(object):
    """
    A "wrapper" class for a map, which maps workspaces from their corresponding
    time regimes.
    """

    def __init__(self):
        self._map = {}

    def addWs(self, ws_name):
        """
        Takes in the given workspace and lists it alongside its time regime
        value.  If the time regime has yet to be created, it will create it,
        and if there is already a workspace listed beside the time regime, then
        the new ws will be appended to that list.
        """
        ws = mtd[ws_name]

        # Get the time regime of the workspace, and use it to find the DRange.
        time_regime = ws.dataX(0)[0]
        time_regimes = time_regime_to_d_range.keys()
        time_regimes.sort()

        for idx in range(len(time_regimes)):
            if idx == len(time_regimes) - 1:
                if time_regimes[idx] < time_regime:
                    time_regime = time_regimes[idx]
                    break
            else:
                if time_regimes[idx] < time_regime < time_regimes[idx + 1]:
                    time_regime = time_regimes[idx]
                    break

        d_range = time_regime_to_d_range[time_regime]
        logger.information('dRange for workspace %s is %s' % (ws_name, str(d_range)))

        # Add the workspace to the map, alongside its DRange.
        if d_range not in self._map:
            self._map[d_range] = [ws_name]
        else:
            #check if x ranges matchs and existing run
            for ws_name in self._map[d_range]:
                map_lastx = mtd[ws_name].readX(0)[-1]
                ws_lastx = ws.readX(0)[-1]

                #if it matches ignore it
                if map_lastx == ws_lastx:
                    DeleteWorkspace(ws)
                    return

            self._map[d_range].append(ws_name)

    def setItem(self, d_range, ws_name):
        """
        Set a dRange and corresponding *single* ws.
        """
        self._map[d_range] = ws_name

    def getMap(self):
        """
        Get access to wrapped map.
        """
        return self._map


def average_ws_list(wsList):
    """
    Returns the average of a list of workspaces.
    """
    # Assert we have some ws in the list, and if there is only one then return it.
    if len(wsList) == 0:
        raise RuntimeError("getAverageWs: Trying to take an average of nothing")

    if len(wsList) == 1:
        return wsList[0]

    # Generate the final name of the averaged workspace.
    avName = "avg"
    for name in wsList:
        avName += "_" + name

    numWorkspaces = len(wsList)

    # Compute the average and put into "__temp_avg".
    __temp_avg = mtd[wsList[0]]
    for i in range(1, numWorkspaces):
        __temp_avg += mtd[wsList[i]]

    __temp_avg /= numWorkspaces

    # Rename the average ws and return it.
    RenameWorkspace(InputWorkspace=__temp_avg, OutputWorkspace=avName)
    return avName


def find_intersection_of_ranges(rangeA, rangeB):
    if rangeA[0] >= rangeA[1] or rangeB[0] >= rangeB[1]:
        raise RuntimeError("Malformed range")

    if rangeA[0] <= rangeA[1] <= rangeB[0] <= rangeB[1]:
        return
    if rangeB[0] <= rangeB[1] <= rangeA[0] <= rangeA[1]:
        return
    if rangeA[0] <= rangeB[0] <= rangeB[1] <= rangeA[1]:
        return rangeB
    if rangeB[0] <= rangeA[0] <= rangeA[1] <= rangeB[1]:
        return rangeA
    if rangeA[0] <= rangeB[0] <= rangeA[1] <= rangeB[1]:
        return [rangeB[0], rangeA[1]]
    if rangeB[0] <= rangeA[0] <= rangeB[1] <= rangeA[1]:
        return [rangeA[0], rangeB[1]]

    # Should never reach here
    raise RuntimeError()


def get_intersetcion_of_ranges(range_list):
    """
    Get the intersections of a list of ranges.  For example, given the ranges:
    [1, 3], [3, 5] and [4, 6], the intersections would be a single range of [4,
    5].

    NOTE: Assumes that no more than a maximum of two ranges will ever cross at
    the same point.  Also, all ranges should obey range[0] <= range[1].
    """
    # Sanity check.
    for myrange in range_list:
        if len(myrange) != 2:
            raise RuntimeError("Unable to find the intersection of a malformed range")

    # Find all combinations of ranges, and see where they intersect.
    rangeCombos = list(itertools.combinations(range_list, 2))
    intersections = []
    for rangePair in rangeCombos:
        intersection = find_intersection_of_ranges(rangePair[0], rangePair[1])
        if intersection is not None:
            intersections.append(intersection)

    # Return the sorted intersections.
    intersections.sort()
    return intersections


def is_in_ranges(range_list, n):
    for myrange in range_list:
        if myrange[0] < n < myrange[1]:
            return True
    return False


class OSIRISDiffractionReduction(PythonAlgorithm):
    """
    Handles the reduction of OSIRIS Diffraction Data.
    """

    _cal = None
    _output_ws_name = None
    _sample_runs = None
    _vanadium_runs = None
    _sam_ws_map = None
    _van_ws_map = None


    def category(self):
        return 'Diffraction;PythonAlgorithms'

    def summary(self):
        return "This Python algorithm performs the operations necessary for the reduction of diffraction data "+\
               "from the Osiris instrument at ISIS "+\
               "into dSpacing, by correcting for the monitor and linking the various d-ranges together."


    def PyInit(self):
        runs_desc='The list of run numbers that are part of the sample run. '+\
                  'There should be five of these in most cases. Enter them as comma separated values.'

        self.declareProperty('Sample', '', doc=runs_desc)
        self.declareProperty('Vanadium', '', doc=runs_desc)

        self.declareProperty(FileProperty('CalFile', '', action=FileAction.Load),
                             doc='Filename of the .cal file to use in the [[AlignDetectors]] and '+\
                                 '[[DiffractionFocussing]] child algorithms.')
        self.declareProperty(MatrixWorkspaceProperty('OutputWorkspace', '', Direction.Output),
                             doc="Name to give the output workspace. If no name is provided, "+\
                                 "one will be generated based on the run numbers.")

        self._cal = None
        self._output_ws_name = None

        self._sam_ws_map = DRangeToWsMap()
        self._van_ws_map = DRangeToWsMap()


    def PyExec(self):
        # Set OSIRIS as default instrument.
        # config["default.instrument"] = 'OSIRIS'
        self._cal = self.getProperty("CalFile").value
        self._output_ws_name = self.getPropertyValue("OutputWorkspace")

        self._sample_runs = self._find_runs(self.getPropertyValue("Sample"))
        self._vanadium_runs = self._find_runs(self.getPropertyValue("Vanadium"))

        self.execDiffOnly()


    #pylint: disable=too-many-branches
    def execDiffOnly(self):
        """
        Execute the algorithm in diffraction-only mode
        """
        # Load all sample and vanadium files, and add the resulting workspaces to the DRangeToWsMaps.
        for fileName in self._sample_runs + self._vanadium_runs:
            Load(Filename=fileName,
                 OutputWorkspace=fileName,
                 SpectrumMin=3)
                 # SpectrumMax=962)

        for sam in self._sample_runs:
            self._sam_ws_map.addWs(sam)
        for van in self._vanadium_runs:
            self._van_ws_map.addWs(van)

        # Check to make sure that there are corresponding vanadium files with the same DRange for each sample file.
        for dRange in self._sam_ws_map.getMap().iterkeys():
            if dRange not in self._van_ws_map.getMap():
                raise RuntimeError("There is no van file that covers the " + str(dRange) + " DRange.")

        # Average together any sample workspaces with the same DRange.  This will mean our map of DRanges
        # to list of workspaces becomes a map of DRanges, each to a *single* workspace.
        tempSamMap = DRangeToWsMap()
        for dRange, wsList in self._sam_ws_map.getMap().iteritems():
            tempSamMap.setItem(dRange, average_ws_list(wsList))
        self._sam_ws_map = tempSamMap

        # Now do the same to the vanadium workspaces.
        tempVanMap = DRangeToWsMap()
        for dRange, wsList in self._van_ws_map.getMap().iteritems():
            tempVanMap.setItem(dRange, average_ws_list(wsList))
        self._van_ws_map = tempVanMap

        # Run necessary algorithms on BOTH the Vanadium and Sample workspaces.
        for dRange, ws in self._sam_ws_map.getMap().items() + self._van_ws_map.getMap().items():
            NormaliseByCurrent(InputWorkspace=ws,
                               OutputWorkspace=ws)
            AlignDetectors(InputWorkspace=ws,
                           OutputWorkspace=ws,
                           CalibrationFile=self._cal)
            DiffractionFocussing(InputWorkspace=ws,
                                 OutputWorkspace=ws,
                                 GroupingFileName=self._cal)
            CropWorkspace(InputWorkspace=ws,
                          OutputWorkspace=ws,
                          XMin=dRange[0],
                          XMax=dRange[1])

        # Divide all sample files by the corresponding vanadium files.
        for dRange in self._sam_ws_map.getMap().iterkeys():
            samWs = self._sam_ws_map.getMap()[dRange]
            vanWs = self._van_ws_map.getMap()[dRange]
            samWs, vanWs = self._rebin_to_smallest(samWs, vanWs)
            Divide(LHSWorkspace=samWs,
                   RHSWorkspace=vanWs,
                   OutputWorkspace=samWs)
            ReplaceSpecialValues(InputWorkspace=samWs,
                                 OutputWorkspace=samWs,
                                 NaNValue=0.0,
                                 InfinityValue=0.0)

        # Create a list of sample workspace NAMES, since we need this for MergeRuns.
        samWsNamesList = []
        for sam in self._sam_ws_map.getMap().itervalues():
            samWsNamesList.append(sam)

        if len(samWsNamesList) > 1:
            # Merge the sample files into one.
            MergeRuns(InputWorkspaces=samWsNamesList,
                      OutputWorkspace=self._output_ws_name)
            for name in samWsNamesList:
                DeleteWorkspace(Workspace=name)
        else:
            RenameWorkspace(InputWorkspace=samWsNamesList[0],
                            OutputWorkspace=self._output_ws_name)

        result = mtd[self._output_ws_name]

        # Create scalar data to cope with where merge has combined overlapping data.
        intersections = get_intersetcion_of_ranges(self._sam_ws_map.getMap().keys())

        dataX = result.dataX(0)
        dataY = []
        dataE = []
        for i in range(0, len(dataX)-1):
            x = ( dataX[i] + dataX[i+1] ) / 2.0
            if is_in_ranges(intersections, x):
                dataY.append(2)
                dataE.append(2)
            else:
                dataY.append(1)
                dataE.append(1)

        # apply scalar data to result workspace
        for i in range(0, result.getNumberHistograms()):
            resultY = result.dataY(i)
            resultE = result.dataE(i)

            resultY = resultY / dataY
            resultE = resultE / dataE

            result.setY(i,resultY)
            result.setE(i,resultE)

        # Delete all workspaces we've created, except the result.
        for ws in self._van_ws_map.getMap().values():
            DeleteWorkspace(Workspace=ws)

        self.setProperty("OutputWorkspace", result)


    def _find_runs(self, run_str):
        """
        Use the FileFinder to find search for the runs given by the string of
        comma-separated run numbers.

        @param run_str A string of run numbers to find
        @returns A list of filepaths
        """
        runs = run_str.split(",")
        run_files = []
        for run in runs:
            try:
                run_files.append(FileFinder.findRuns(run)[0])
            except IndexError:
                raise RuntimeError("Could not locate sample file: " + run)

        return run_files


    def _rebin_to_smallest(self, samWS, vanWS):
        """
        At some point a change to the control program meant that the raw data
        got an extra bin. This prevents runs past this point being normalised
        with a vanadium from an earlier point.  Here we simply rebin to the
        smallest workspace if the sizes don't match

        @param samWS A workspace object containing the sample run
        @param vanWS A workspace object containing the vanadium run
        @returns samWS, vanWS rebinned  to the smallest if necessary
        """
        sample_size, van_size = mtd[samWS].blocksize(), mtd[vanWS].blocksize()
        if sample_size == van_size:
            return samWS, vanWS

        if sample_size < van_size:
            # Rebin vanadium to match sample
            RebinToWorkspace(WorkspaceToRebin=vanWS,
                             WorkspaceToMatch=samWS,
                             OutputWorkspace=vanWS)
        else:
            # Rebin sample to match vanadium
            RebinToWorkspace(WorkspaceToRebin=samWS,
                             WorkspaceToMatch=vanWS,
                             OutputWorkspace=samWS)

        return samWS, vanWS


AlgorithmFactory.subscribe(OSIRISDiffractionReduction)
