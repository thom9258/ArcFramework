#pragma once

namespace ArcGraphics {

class IsNotLvalueCopyable 
{
public:
    IsNotLvalueCopyable(IsNotLvalueCopyable&&) = delete;
    IsNotLvalueCopyable operator=(IsNotLvalueCopyable&&) = delete;

protected:
    IsNotLvalueCopyable() = default;
    virtual ~IsNotLvalueCopyable() = default;
};

class IsNotRvalueCopyable 
{
public:
    IsNotRvalueCopyable(IsNotRvalueCopyable&&) = delete;
    IsNotRvalueCopyable operator=(IsNotRvalueCopyable&&) = delete;

protected:
    IsNotRvalueCopyable() = default;
    virtual ~IsNotRvalueCopyable() = default;
};

}
