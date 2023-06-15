 #include <iostream>

 static void debug(int a, int b, std::string c , std::string d) {
        std::cout << "\n" << c << a << "," << b << d << std::endl;
    }

    static void debug(float a, float b, std::string c[] , std::string d[] ) {
        std::cout << "\n" << c << a << "," << b << d << std::endl;
    }

    static void debug(std::string c) {
        std::cout << "\n" << c << std::endl;
    }