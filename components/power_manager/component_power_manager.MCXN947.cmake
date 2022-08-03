# Add set(CONFIG_USE_component_power_manager true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

if(CONFIG_USE_driver_common)

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/core/fsl_pm_core.c
  ${CMAKE_CURRENT_LIST_DIR}/boards/MCX-N9XX-EVK/fsl_pm_board.c
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}/core
  ${CMAKE_CURRENT_LIST_DIR}/boards/MCX-N9XX-EVK
)

else()

message(SEND_ERROR "component_power_manager.MCXN947 dependency does not meet, please check ${CMAKE_CURRENT_LIST_FILE}.")

endif()
