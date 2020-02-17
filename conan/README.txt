In the following example we assume you have the kdsoap source available in
/path/to/kdsoap-source.  replace '/path/to/kdsoap-source' to fit your needs.

$ conan create -s build_type=Release -o kdsoap:build_examples=True -o kdsoap:build_tests=True /path/to/kdsoap-source/conan kdab/stable

configuration options:
 * build_static
   Builds static versions of the libraries. Default=False

 * build_tests
   Build the test harness. Default=False

 * build_examples
   Build the examples. Default=True
