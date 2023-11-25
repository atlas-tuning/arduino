class Counter {
    public:
        Counter(int window);

        void increment(float value);

        void clear();

        float avg();
        float sum();

        int size();
        int getWindow();

    private:
        int window;
        int count;
        float buffer;
};