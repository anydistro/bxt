configure_file(${LIST_DIR}/index.html.in
                ${BINARY_DIR}/bin/web/swagger/index.html)

configure_file(${LIST_DIR}/swagger-initializer.js.in
                ${BINARY_DIR}/bin/web/swagger/swagger-initializer.js)

configure_file(${LIST_DIR}/openapi.yml.in
                ${BINARY_DIR}/bin/web/swagger/openapi.yml)
