std::fstream file("values.csv", std::ios::out);
    file << " n / k,";
    for(int k = 0; k <= 3; ++k) {
        file << k << (k < 3 ? "," : "\n");
    }
    for(int n = 1; n <= 12; ++n) {
        file << n << ",";
        for(int k = 0; k <= 3; ++k) {
            WysSolver solver(n, k);
            int64_t ans = solver.solve_game();
            std::cout << " n = " << n << ", k = " << k << ", ans is " << ans << "\n";
            file << ans << (k < 3 ? "," : "\n");;
        }
    }