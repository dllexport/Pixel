if (WIN32)
  set(GLSL_VALIDATOR "${VULKAN_SDK}/Bin/glslangValidator.exe")
elseif (APPLE)
  set(GLSL_VALIDATOR "${VULKAN_SDK}/bin/glslangValidator")
endif()

macro(APPEND_GLSL_TO_TARGET target sources)
	foreach(GLSL ${sources})
		get_filename_component(FILE_NAME ${GLSL} NAME)
		set(SPIRV "${CMAKE_CURRENT_BINARY_DIR}/Shaders/${FILE_NAME}.spv")
		add_custom_command(
			OUTPUT ${SPIRV}
			COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/Shaders
			COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
			DEPENDS ${GLSL})
		list(APPEND SPIRV_BINARY_FILES ${SPIRV})
	endforeach(GLSL)

	add_custom_target(
	    ${target}_Shader
	    DEPENDS ${SPIRV_BINARY_FILES}
	)

	add_dependencies(${target} ${target}_Shader)
endmacro()