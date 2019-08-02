#include "Datarefs.h"

std::vector<dataref::dataref_info> dataref::get_list()
{
	return dataref_list_;
}

void dataref::reset_builder() {
	flexbuffers_builder_.Clear();
	flatbuffers_builder_.Clear();
}

// Remove all the dataref in the dataref list
void dataref::empty_list() {
	dataref_list_.clear();
	reset_builder();
	set_status(false);
}

uint8_t* dataref::get_serialized_data()
{
	const auto data = flatbuffers_builder_.CreateVector(get_flexbuffers_data());
	const auto size = get_flexbuffers_size();

	Ditto::DataBuilder data_builder(flatbuffers_builder_);
	data_builder.add_size(size);
	data_builder.add_buffer(data);
	const auto finished = data_builder.Finish();

	flatbuffers_builder_.Finish(finished);

	return flatbuffers_builder_.GetBufferPointer();
}

size_t dataref::get_serialized_size()
{
	return flatbuffers_builder_.GetSize();
}

std::vector<uint8_t> dataref::get_flexbuffers_data() {

	const auto map_start = flexbuffers_builder_.StartMap();

	for (auto& dataref : dataref_list_) {

		// If start and end index does not present that means the dataref is single value dataref
		if (!dataref.start_index.has_value() && !dataref.num_value.has_value()) {
			if (dataref.type == "int") {
				flexbuffers_builder_.Int(dataref.name.c_str(), get_value_int(dataref.dataref));
			}
			else if (dataref.type == "float") {
				flexbuffers_builder_.Float(dataref.name.c_str(), get_value_float(dataref.dataref));
			}
			else if (dataref.type == "double") {
				flexbuffers_builder_.Double(dataref.name.c_str(), get_value_double(dataref.dataref));
			}
		}
		else {
			const auto vector_start = flexbuffers_builder_.StartVector(dataref.name.c_str());
			if (dataref.type == "int") {
				for (auto int_num : get_value_int_array(dataref.dataref, dataref.start_index.value(), dataref.num_value.value())) {
					flexbuffers_builder_.Int(int_num);
				}
			}
			else if (dataref.type == "float") {
				for (auto float_num : get_value_float_array(dataref.dataref, dataref.start_index.value(), dataref.num_value.value())) {
					flexbuffers_builder_.Float(float_num);
				}
			}
			else if (dataref.type == "char") {
				auto str = get_value_char_array(dataref.dataref, dataref.start_index.value(), dataref.num_value.value());
				flexbuffers_builder_.String(std::string(str.begin(), str.end()));
			}
			flexbuffers_builder_.EndVector(vector_start, false, false);
		}
	}

	flexbuffers_builder_.EndMap(map_start);
	flexbuffers_builder_.Finish();

	return flexbuffers_builder_.GetBuffer();
}

bool dataref::get_status() {
	return status_;
}

void dataref::set_status(bool in_status) {
	status_ = in_status;
}

size_t dataref::get_flexbuffers_size() {
	return flexbuffers_builder_.GetSize();
}

void dataref::get_data_list()
{
	try
	{
		const auto input_file = cpptoml::parse_file(plugin_path_ + "Datarefs.toml");
		// Create a list of all the Data table in the toml file
		const auto data_list = input_file->get_table_array("Data");

		// Loop through all the tables
		for (const auto& table : *data_list)
		{
			XPLMDataRef new_dataref = XPLMFindDataRef(table->get_as<std::string>("string").value_or("").c_str());

			auto start = table->get_as<int>("start_index").value_or(-1);
			auto num = table->get_as<int>("num_value").value_or(-1);
			dataref_info temp_dataref_info;

			temp_dataref_info.name = table->get_as<std::string>("name").value_or("");
			temp_dataref_info.dataref = new_dataref;
			temp_dataref_info.type = table->get_as<std::string>("type").value_or("");

			if (start != -1 && num != -1) {
				temp_dataref_info.start_index = start;
				temp_dataref_info.num_value = num;
			}
			else {
				temp_dataref_info.start_index = std::nullopt;
				temp_dataref_info.num_value = std::nullopt;
			}
			dataref_list_.emplace_back(temp_dataref_info);
		}
	}
	catch (const cpptoml::parse_exception& ex)
	{
		XPLMDebugString(ex.what());
	}
}

int dataref::get_value_int(XPLMDataRef in_dataref)
{
	return XPLMGetDatai(in_dataref);
}

float dataref::get_value_float(XPLMDataRef in_dataref) {
	return XPLMGetDataf(in_dataref);
}

double dataref::get_value_double(XPLMDataRef in_dataref) {
	return XPLMGetDatad(in_dataref);
}

std::vector<int> dataref::get_value_int_array(XPLMDataRef in_dataref, int start_index, int number_of_value) {
	std::vector<int> return_val;
	for (auto i = 1; i <= number_of_value; ++i) {
		int temp;
		XPLMGetDatavi(in_dataref, &temp, start_index, 1);
		return_val.emplace_back(temp);
	}
	return return_val;
}

std::vector<float> dataref::get_value_float_array(XPLMDataRef in_dataref, int start_index, int number_of_value) {
	std::vector<float> return_val;

	for (auto i = 1; i <= number_of_value; ++i) {
		float temp;
		XPLMGetDatavf(in_dataref, &temp, start_index, 1);
		return_val.emplace_back(temp);
	}
	return return_val;
}

std::vector<char> dataref::get_value_char_array(XPLMDataRef in_dataref, int start_index, int number_of_value) {
	std::vector<char> return_val;

	for (auto i = 1; i <= number_of_value; ++i) {
		char temp;
		XPLMGetDatab(in_dataref, &temp, start_index, 1);
		return_val.emplace_back(temp);
	}
	return return_val;
}

void dataref::init() {
	get_data_list();
	set_status(true);
}

void dataref::set_plugin_path(const std::string& path)
{
	plugin_path_ = path;
}
