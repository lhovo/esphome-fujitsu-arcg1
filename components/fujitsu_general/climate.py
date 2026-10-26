import esphome.codegen as cg
from esphome.components import climate_ir
import esphome.config_validation as cv
from esphome.types import ConfigType

AUTO_LOAD = ["climate_ir"]

fujitsu_general_ns = cg.esphome_ns.namespace("fujitsu_general")
FujitsuGeneralClimate = fujitsu_general_ns.class_(
    "FujitsuGeneralClimate", climate_ir.ClimateIR
)

FujitsuModel = fujitsu_general_ns.enum("FujitsuModel")
MODELS = {"DEFAULT": FujitsuModel.DEFAULT, "AR_CG1": FujitsuModel.AR_CG1}
def validate_forwarding(config):
    if config["forward_remote_commands"]:
        if str(config["model"]) != "AR_CG1":
            raise cv.Invalid("forward_remote_commands requires model: AR_CG1")
        if "receiver_id" not in config:
            raise cv.Invalid("forward_remote_commands requires receiver_id")
    return config


CONFIG_SCHEMA = cv.All(
    climate_ir.climate_ir_with_receiver_schema(FujitsuGeneralClimate).extend(
        {
            cv.Optional("model", default="DEFAULT"): cv.enum(MODELS, upper=True),
            cv.Optional("forward_remote_commands", default=False): cv.boolean,
        }
    ),
    validate_forwarding,
)


async def to_code(config: ConfigType) -> None:
    var = await climate_ir.new_climate_ir(config)
    cg.add(var.set_model(config["model"]))
    cg.add(var.set_forward_remote_commands(config["forward_remote_commands"]))
