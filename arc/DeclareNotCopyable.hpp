#pragma once


namespace arc {

class DeclareNotCopyable {
public:
    DeclareNotCopyable(DeclareNotCopyable&&) = delete;
    DeclareNotCopyable(const DeclareNotCopyable&) = delete;
    DeclareNotCopyable operator=(DeclareNotCopyable&&) = delete;
    DeclareNotCopyable operator=(const DeclareNotCopyable&) = delete;

protected:
    DeclareNotCopyable() = default;
    virtual ~DeclareNotCopyable() = default;
};

}
