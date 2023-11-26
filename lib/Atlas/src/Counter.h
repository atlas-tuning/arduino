class Counter {
    public:
        Counter(int window);

        void increment(double value);

        void clear();

        double avg();
        double sum();

        int size();
        int getWindow();

    private:
        int window;
        int count;
        double buffer;
};