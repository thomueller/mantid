from __future__ import (absolute_import, division, print_function)

import mantid.simpleapi as mantid

from isis_powder.routines import common, InstrumentSettings, yaml_parser
from isis_powder.abstract_inst import AbstractInst
from isis_powder.pearl_routines import pearl_algs, pearl_output, pearl_advanced_config, pearl_param_mapping


class Pearl(AbstractInst):

    def __init__(self, **kwargs):
        basic_config_dict = yaml_parser.open_yaml_file_as_dictionary(kwargs.get("config_file", None))

        self._inst_settings = InstrumentSettings.InstrumentSettings(
           param_map=pearl_param_mapping.attr_mapping, adv_conf_dict=pearl_advanced_config.get_all_adv_variables(),
           basic_conf_dict=basic_config_dict, kwargs=kwargs)

        super(Pearl, self).__init__(user_name=self._inst_settings.user_name,
                                    calibration_dir=self._inst_settings.calibration_dir,
                                    output_dir=self._inst_settings.output_dir, inst_prefix="PEARL")

        self._cached_run_details = None
        self._cached_run_details_number = None

    def focus(self, **kwargs):
        self._switch_long_mode_inst_settings(kwargs.get("long_mode"))
        self._inst_settings.update_attributes(kwargs=kwargs)
        return self._focus(run_number_string=self._inst_settings.run_number,
                           do_van_normalisation=self._inst_settings.van_norm)

    def create_vanadium(self, **kwargs):
        self._switch_long_mode_inst_settings(kwargs.get("long_mode"))
        kwargs["perform_attenuation"] = None  # Hard code this off as we do not need an attenuation file
        self._inst_settings.update_attributes(kwargs=kwargs)

        if str(self._inst_settings.tt_mode).lower() == "all":
            for new_tt_mode in ["tt35", "tt70", "tt88"]:
                self._inst_settings.tt_mode = new_tt_mode
                self._run_create_vanadium()
        else:
            self._run_create_vanadium()

    def _run_create_vanadium(self):
        # Provides a minimal wrapper so if we have tt_mode 'all' we can loop round
        run_details = self._get_run_details(run_number_string=self._inst_settings.run_in_range)
        run_details.run_number = run_details.vanadium_run_numbers
        return self._create_vanadium(run_details=run_details,
                                     do_absorb_corrections=self._inst_settings.absorb_corrections)

    def _get_run_details(self, run_number_string):
        if self._cached_run_details_number == run_number_string:
            return self._cached_run_details

        run_details = pearl_algs.get_run_details(run_number_string=run_number_string, inst_settings=self._inst_settings)

        self._cached_run_details_number = run_number_string
        self._cached_run_details = run_details
        return run_details

    # Params #

    @staticmethod
    def _generate_input_file_name(run_number):
        return _generate_inst_padding(run_number=run_number)

    def _generate_output_file_name(self, run_number_string):
        inst = self._inst_settings
        return pearl_algs.generate_out_name(run_number_string=run_number_string, absorb_on=inst.absorb_corrections,
                                            long_mode_on=inst.long_mode, tt_mode=inst.tt_mode)

    def _attenuate_workspace(self, input_workspace):
        attenuation_path = self._inst_settings.attenuation_file_path
        return pearl_algs.attenuate_workspace(attenuation_file_path=attenuation_path, ws_to_correct=input_workspace)

    def _normalise_ws_current(self, ws_to_correct, run_details=None):
        monitor_ws = common.get_monitor_ws(ws_to_process=ws_to_correct, run_number_string=run_details.run_number,
                                           instrument=self)
        normalised_ws = pearl_algs.normalise_ws_current(ws_to_correct=ws_to_correct, monitor_ws=monitor_ws,
                                                        spline_coeff=self._inst_settings.monitor_spline,
                                                        integration_range=self._inst_settings.monitor_integration_range,
                                                        lambda_values=self._inst_settings.monitor_lambda)
        common.remove_intermediate_workspace(monitor_ws)
        return normalised_ws

    def _generate_auto_vanadium_calibration(self, run_details):
        # The instrument scientists prefer everything to be explicit on this instrument so
        # instead we don't try to run this automatically
        raise NotImplementedError("You must run the create_vanadium method manually on Pearl")

    def _get_monitor_spectra_index(self, run_number):
        return self._inst_settings.monitor_spec_no

    def _spline_vanadium_ws(self, focused_vanadium_spectra):
        focused_vanadium_spectra = pearl_algs.strip_bragg_peaks(focused_vanadium_spectra)
        return common.spline_workspaces(focused_vanadium_spectra=focused_vanadium_spectra,
                                        num_splines=self._inst_settings.spline_coefficient)

    def _output_focused_ws(self, processed_spectra, run_details, output_mode=None):
        if not output_mode:
            output_mode = self._inst_settings.focus_mode
        output_spectra = \
            pearl_output.generate_and_save_focus_output(self, processed_spectra=processed_spectra,
                                                        run_details=run_details, focus_mode=output_mode,
                                                        perform_attenuation=self._inst_settings.perform_atten)
        group_name = "PEARL" + str(run_details.user_input_run_number)
        group_name += '_' + self._inst_settings.tt_mode + "-Results-D-Grp"
        grouped_d_spacing = mantid.GroupWorkspaces(InputWorkspaces=output_spectra, OutputWorkspace=group_name)
        return grouped_d_spacing, None

    def _crop_banks_to_user_tof(self, focused_banks):
        return common.crop_banks_using_crop_list(focused_banks, self._inst_settings.tof_cropping_values)

    def _crop_raw_to_expected_tof_range(self, ws_to_crop):
        out_ws = common.crop_in_tof(ws_to_crop=ws_to_crop, x_min=self._inst_settings.raw_data_crop_vals[0],
                                    x_max=self._inst_settings.raw_data_crop_vals[-1])
        return out_ws

    def _crop_van_to_expected_tof_range(self, van_ws_to_crop):
        cropped_ws = common.crop_in_tof(ws_to_crop=van_ws_to_crop, x_min=self._inst_settings.van_tof_cropping[0],
                                        x_max=self._inst_settings.van_tof_cropping[-1])
        return cropped_ws

    def _apply_absorb_corrections(self, run_details, van_ws):
        # TODO move this to an instrument param
        gen_absorb = False
        if gen_absorb:
            pearl_algs.generate_vanadium_absorb_corrections(van_ws=van_ws)

        return pearl_algs.apply_vanadium_absorb_corrections(van_ws=van_ws, run_details=run_details)

    def _switch_long_mode_inst_settings(self, long_mode_on):
        self._inst_settings.update_attributes(advanced_config=pearl_advanced_config.get_long_mode_dict(long_mode_on),
                                              suppress_warnings=True)


def _generate_inst_padding(run_number):
    digit = len(str(run_number))

    number_of_digits = 8
    filename = "PEARL"

    for i in range(0, number_of_digits - digit):
        filename += "0"

    filename += str(run_number)
    return filename
