namespace arc {

class DeclareNotCopyable {
public:
    DeclareNotCopyable() = default;
    ~DeclareNotCopyable() = default;
    DeclareNotCopyable(DeclareNotCopyable&&) = delete;
    DeclareNotCopyable(const DeclareNotCopyable&) = delete;
    DeclareNotCopyable operator=(DeclareNotCopyable&&) = delete;
    DeclareNotCopyable operator=(const DeclareNotCopyable&) = delete;
};

}
