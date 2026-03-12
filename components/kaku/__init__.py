import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import remote_base

from esphome.const import (
    CONF_ADDRESS,
    CONF_PERIOD,
    CONF_SWITCH_TYPE,
    CONF_DIM_LEVEL_PRESENT,
    CONF_DIM_LEVEL,
    CONF_UNIT,
    CONF_GROUP,
)

AUTO_LOAD = ["remote_base"]

CODEOWNERS = ["@YOUR_GITHUB_USERNAME"]

# Empty component schema - just used to trigger early initialization
CONFIG_SCHEMA = cv.Schema({})

# Declare the KAKU protocol classes
KakuData, KakuBinarySensor, KakuTrigger, KakuAction, KakuDumper = remote_base.declare_protocol("Kaku")

KAKU_SCHEMA = cv.Schema(
    {
        cv.Optional(CONF_PERIOD, default=0): cv.uint32_t,
        cv.Required(CONF_ADDRESS): cv.hex_uint32_t,
        cv.Optional(CONF_GROUP, default=False): cv.boolean,
        cv.Optional(CONF_SWITCH_TYPE, default=0): cv.uint8_t,
        cv.Optional(CONF_UNIT, default=0): cv.uint8_t,
        cv.Optional(CONF_DIM_LEVEL_PRESENT, default=False): cv.boolean,
        cv.Optional(CONF_DIM_LEVEL, default=0): cv.uint8_t,
    }
)


@remote_base.register_binary_sensor("kaku", KakuBinarySensor, KAKU_SCHEMA)
def kaku_binary_sensor(var, config):
    cg.add(
        var.set_data(
            cg.StructInitializer(
                KakuData,
                ("period", config[CONF_PERIOD]),
                ("address", config[CONF_ADDRESS]),
                ("group", config[CONF_GROUP]),
                ("switchType", config[CONF_SWITCH_TYPE]),
                ("unit", config[CONF_UNIT]),
                ("dimLevelPresent", config[CONF_DIM_LEVEL_PRESENT]),
                ("dimLevel", config[CONF_DIM_LEVEL]),
            )
        )
    )


@remote_base.register_trigger("kaku", KakuTrigger, KakuData)
def kaku_trigger(var, config):
    pass


@remote_base.register_dumper("kaku", KakuDumper)
def kaku_dumper(var, config):
    pass


@remote_base.register_action("kaku", KakuAction, KAKU_SCHEMA)
async def kaku_action(var, config, args):
    cg.add(var.set_period(await cg.templatable(config[CONF_PERIOD], args, cg.uint32)))
    cg.add(var.set_address(await cg.templatable(config[CONF_ADDRESS], args, cg.uint32)))
    cg.add(var.set_group(await cg.templatable(config[CONF_GROUP], args, cg.bool_)))
    cg.add(var.set_switch_type(await cg.templatable(config[CONF_SWITCH_TYPE], args, cg.uint8)))
    cg.add(var.set_unit(await cg.templatable(config[CONF_UNIT], args, cg.uint8)))
    cg.add(
        var.set_dim_level_present(
            await cg.templatable(config[CONF_DIM_LEVEL_PRESENT], args, cg.bool_)
        )
    )
    cg.add(var.set_dim_level(await cg.templatable(config[CONF_DIM_LEVEL], args, cg.uint8)))
