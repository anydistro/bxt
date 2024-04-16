configure_file(${LIST_DIR}/index.html.in
                ${BINARY_DIR}/bin/frontend/swagger/index.html)

configure_file(${LIST_DIR}/swagger-initializer.js.in
                ${BINARY_DIR}/bin/frontend/swagger/swagger-initializer.js)

configure_file(${LIST_DIR}/openapi.yml.in
                ${BINARY_DIR}/bin/frontend/swagger/openapi.yml)
