#include <AgIsoStack.hpp>

using namespace isobus;

auto can0 = std::make_shared<FlexCANT4Plugin>(0);
std::shared_ptr<InternalControlFunction>  ISOBUSControlFunction = nullptr;
std::shared_ptr<DiagnosticProtocol> ISOBUSDiagnostics = nullptr;
std::shared_ptr<SpeedMessagesInterface> ISOBUSSpeedInterface = nullptr;

// A log sink for the CAN stack
class CustomLogger : public CANStackLogger
{
public:
	void sink_CAN_stack_log(CANStackLogger::LoggingLevel level, const std::string &text) override
	{
		switch (level)
		{
			case LoggingLevel::Debug:
			{
				Serial.print("[Debug]: ");
			}
			break;

			case LoggingLevel::Info:
			{
				Serial.print("[Info]: ");
			}
			break;

			case LoggingLevel::Warning:
			{
				Serial.print("[Warning]: ");
			}
			break;

			case LoggingLevel::Error:
			{
				Serial.print("[Error]: ");
			}
			break;

			case LoggingLevel::Critical:
			{
        		Serial.print("[Critical]: ");
			}
			break;
		}
		 Serial.println(text.c_str());
	}
};

static CustomLogger logger;

// These are optional callbacks you can use to do something as soon as new speed information is received.
static void test_mss_callback(const std::shared_ptr<SpeedMessagesInterface::MachineSelectedSpeedData> machineSpeedData, bool)
{
  if (nullptr != machineSpeedData)
  {
	  Serial.write(("[MSS] " + isobus::to_string(machineSpeedData->get_machine_speed()) + "mm/s\n").c_str());
  }
}

static void test_wbs_callback(const std::shared_ptr<SpeedMessagesInterface::WheelBasedMachineSpeedData> wheelSpeedData, bool)
{
	if (nullptr != wheelSpeedData)
  {
	  Serial.write(("[WBS] " + isobus::to_string(wheelSpeedData->get_machine_speed()) + "mm/s\n").c_str());
  }
}

static void test_gbs_callback(const std::shared_ptr<SpeedMessagesInterface::GroundBasedSpeedData> groundSpeedData, bool)
{
	if (nullptr != groundSpeedData)
  {
    Serial.write(("[GBS] " + isobus::to_string(groundSpeedData->get_machine_speed()) + "mm/s\n").c_str());
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  CANStackLogger::set_can_stack_logger_sink(&logger);
  CANStackLogger::set_log_level(isobus::CANStackLogger::LoggingLevel::Debug);
  // Optional, add delay() here to give you time to connect to the serial logger
  CANHardwareInterface::set_number_of_can_channels(1);
  CANHardwareInterface::assign_can_channel_frame_handler(0, can0);
  CANHardwareInterface::start();
  CANHardwareInterface::update();

  NAME deviceNAME(0);
  // Make sure you change these for your device
  // This is an example device that is using a manufacturer code that is currently unused at time of writing
  deviceNAME.set_arbitrary_address_capable(true);
  deviceNAME.set_industry_group(0);
  deviceNAME.set_device_class(0);
  deviceNAME.set_function_code(static_cast<std::uint8_t>(isobus::NAME::Function::IOController));
  deviceNAME.set_identity_number(2);
  deviceNAME.set_ecu_instance(0);
  deviceNAME.set_function_instance(0);
  deviceNAME.set_device_class_instance(0);
  deviceNAME.set_manufacturer_code(1407); // This is the Open-Agriculture manufacturer code. You are welcome to use it if you want.
  // If you want to set a preferred address, you can add another parameter below, like (deviceNAME, 0, 0x81), otherwise the CAN stack will choose one automatically.
  ISOBUSControlFunction = CANNetworkManager::CANNetwork.create_internal_control_function(deviceNAME, 0);
  ISOBUSDiagnostics = std::make_shared<DiagnosticProtocol>(ISOBUSControlFunction);
  ISOBUSDiagnostics->initialize();

  // Change these to be specific to your device
  ISOBUSDiagnostics->set_product_identification_brand("Arduino");
  ISOBUSDiagnostics->set_product_identification_code("123456789");
  ISOBUSDiagnostics->set_product_identification_model("Example");
  ISOBUSDiagnostics->set_software_id_field(0, "0.0.1");
  ISOBUSDiagnostics->set_ecu_id_field(DiagnosticProtocol::ECUIdentificationFields::HardwareID, "Hardware ID");
  ISOBUSDiagnostics->set_ecu_id_field(DiagnosticProtocol::ECUIdentificationFields::Location, "The Aether");
  ISOBUSDiagnostics->set_ecu_id_field(DiagnosticProtocol::ECUIdentificationFields::ManufacturerName, "None");
  ISOBUSDiagnostics->set_ecu_id_field(DiagnosticProtocol::ECUIdentificationFields::PartNumber, "1234");
  ISOBUSDiagnostics->set_ecu_id_field(DiagnosticProtocol::ECUIdentificationFields::SerialNumber, "1");
  ISOBUSDiagnostics->set_ecu_id_field(DiagnosticProtocol::ECUIdentificationFields::Type, "AgISOStack");

  // Set up to receive speed messages.
  ISOBUSSpeedInterface = std::make_shared<SpeedMessagesInterface>(nullptr);
  ISOBUSSpeedInterface->initialize();
  ISOBUSSpeedInterface->get_machine_selected_speed_data_event_publisher().add_listener(test_mss_callback);
  ISOBUSSpeedInterface->get_ground_based_machine_speed_data_event_publisher().add_listener(test_gbs_callback);
  ISOBUSSpeedInterface->get_wheel_based_machine_speed_data_event_publisher().add_listener(test_wbs_callback);
}

void loop() {
  // put your main code here, to run repeatedly:
  static std::uint32_t bestSpeedPrintTimestamp = SystemTiming::get_timestamp_ms();

  ISOBUSDiagnostics->update(); // Update diagnostics interface
  ISOBUSSpeedInterface->update();
  CANHardwareInterface::update(); // Update CAN stack

  // Every 1 second, print the "best speed".
  if (SystemTiming::time_expired_ms(bestSpeedPrintTimestamp, 1000))
  {
    std::uint32_t bestSpeed = 0;

    if (ISOBUSSpeedInterface->get_number_received_machine_selected_speed_sources() > 0)
    {
      bestSpeed = ISOBUSSpeedInterface->get_received_machine_selected_speed(0)->get_machine_speed();
      Serial.write("Best speed is MSS: ");
      Serial.write((isobus::to_string(bestSpeed) + "mm/s\n").c_str());
    }
    else if (ISOBUSSpeedInterface->get_number_received_ground_based_speed_sources() > 0)
    {
      bestSpeed = ISOBUSSpeedInterface->get_received_ground_based_speed(0)->get_machine_speed();
      Serial.write("Best speed is GBS: ");
      Serial.write((isobus::to_string(bestSpeed) + "mm/s\n").c_str());
    }
    else if (ISOBUSSpeedInterface->get_number_received_wheel_based_speed_sources() > 0)
    {
      bestSpeed = ISOBUSSpeedInterface->get_received_wheel_based_speed(0)->get_machine_speed();
      Serial.write("Best speed is WBS: ");
      Serial.write((isobus::to_string(bestSpeed) + "mm/s\n").c_str());
    }
    else
    {
      Serial.write("No valid speed sources.\n");
    }
    bestSpeedPrintTimestamp = SystemTiming::get_timestamp_ms();
  }
}
