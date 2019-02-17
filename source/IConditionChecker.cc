#include "headers/IConditionChecker.hh"

IConditionChecker::IConditionChecker() {
    this->ID = this->ICCID;

    this->thread_enabled = false;
    this->thread_terminate = false;
}

IConditionChecker::~IConditionChecker() {

}