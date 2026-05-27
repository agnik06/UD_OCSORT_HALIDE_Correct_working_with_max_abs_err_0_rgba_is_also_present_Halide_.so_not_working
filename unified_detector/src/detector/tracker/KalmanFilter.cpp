// #include "include/KalmanFilter.hpp"
// #include <iostream>
// namespace ocsort {
//     KalmanFilterNew::KalmanFilterNew() {};
//     KalmanFilterNew::KalmanFilterNew(int dim_x_, int dim_z_) {
//         // dim_x_ and dim_z_ are accepted for API compatibility but ignored:
//         // matrices are now fixed at 7 and 4 (compile-time).
//         (void)dim_x_; (void)dim_z_;
//         x.setZero();
//         P.setIdentity();
//         Q.setIdentity();
//         B.setIdentity();
//         F.setIdentity();
//         H.setZero();
//         R.setIdentity();
//         M.setZero();
//         z.setZero();
//         K.setZero();
//         y.setZero();
//         S.setZero();
//         SI.setZero();

//         x_prior = x;
//         P_prior = P;
//         x_post = x;
//         P_post = P;
//     };
//     void KalmanFilterNew::predict() {
//         x = F * x;
//         P = _alpha_sq * F * P * F.transpose() + Q;
//         x_prior = x;
//         P_prior = P;
//     }
//     void KalmanFilterNew::update(Eigen::VectorXf* z_) {
//         if (z_ == nullptr) {
//             history_obs.push_back(nullptr);
//         } else {
//             history_obs.push_back(std::make_shared<Eigen::VectorXf>(*z_));
//         }
//         // Cap history_obs growth: unfreeze() only ever needs the last few non-null
//         // entries, and max_age=30 means a tracker is erased long before this matters.
//         // 100 gives ample headroom and prevents unbounded memory growth on long videos.
//         constexpr size_t HISTORY_OBS_CAP = 100;
//         if (history_obs.size() > HISTORY_OBS_CAP) {
//             history_obs.erase(history_obs.begin(),
//                               history_obs.begin() + (history_obs.size() - HISTORY_OBS_CAP));
//         }
//         if (z_ == nullptr) {
//             if (true == observed) freeze();
//             observed = false;
//             z.setZero();
//             x_post = x;
//             P_post = P;
//             y.setZero();
//             return;
//         }
//         if (false == observed) unfreeze();
//         observed = true;
//         // *z_ is dynamic; copy into fixed-size measurement vector.
//         Eigen::Matrix<float, 4, 1> z_fixed = *z_;
//         y = z_fixed - H * x;
//         Eigen::Matrix<float, 7, 4> PHT = P * H.transpose();
//         S = H * PHT + R;
//         SI = S.inverse();
//         K = PHT * SI;
//         x = x + K * y;
//         Eigen::Matrix<float, 7, 7> I_KH = I - K * H;
//         P = ((I_KH * P) * I_KH.transpose()) + ((K * R) * K.transpose());
//         z = z_fixed;
//         x_post = x;
//         P_post = P;
//     }
//     void KalmanFilterNew::freeze() {
//         attr_saved.IsInitialized = true;
//         attr_saved.x = x;
//         attr_saved.P = P;
//         attr_saved.Q = Q;
//         attr_saved.B = B;
//         attr_saved.F = F;
//         attr_saved.H = H;
//         attr_saved.R = R;
//         attr_saved._alpha_sq = _alpha_sq;
//         attr_saved.M = M;
//         attr_saved.z = z;
//         attr_saved.K = K;
//         attr_saved.y = y;
//         attr_saved.S = S;
//         attr_saved.SI = SI;
//         attr_saved.x_prior = x_prior;
//         attr_saved.P_prior = P_prior;
//         attr_saved.x_post = x_post;
//         attr_saved.P_post = P_post;
//         attr_saved.history_obs = history_obs;
//     }
//     void KalmanFilterNew::unfreeze() {
//         if (true == attr_saved.IsInitialized) {
//             new_history = history_obs;
//             x = attr_saved.x;
//             P = attr_saved.P;
//             Q = attr_saved.Q;
//             B = attr_saved.B;
//             F = attr_saved.F;
//             H = attr_saved.H;
//             R = attr_saved.R;
//             _alpha_sq = attr_saved._alpha_sq;
//             M = attr_saved.M;
//             z = attr_saved.z;
//             K = attr_saved.K;
//             y = attr_saved.y;
//             S = attr_saved.S;
//             SI = attr_saved.SI;
//             x_prior = attr_saved.x_prior;
//             P_prior = attr_saved.P_prior;
//             x_post = attr_saved.x_post;
//             history_obs.erase(history_obs.end() - 1);
//             Eigen::VectorXf box1;           
//             Eigen::VectorXf box2;           
//             int lastNotNullIndex = -1;     
//             int secondLastNotNullIndex = -1;
//             for (int i = new_history.size() - 1; i >= 0; i--) {
//                 if (new_history[i] != nullptr) {
//                     if (lastNotNullIndex == -1) {
//                         lastNotNullIndex = i;
//                         box2 = *(new_history.at(lastNotNullIndex));
//                     }
//                     else if (secondLastNotNullIndex == -1) {
//                         secondLastNotNullIndex = i;
//                         box1 = *(new_history.at(secondLastNotNullIndex));
//                         break;
//                     }
//                 }
//             }
//             if (secondLastNotNullIndex == -1 || lastNotNullIndex == -1) {
//                 return;
//             }
//             double time_gap = lastNotNullIndex - secondLastNotNullIndex;
//             double x1 = box1[0];
//             double x2 = box2[0];
//             double y1 = box1[1];
//             double y2 = box2[1];
//             double w1 = std::sqrt(box1[2] * box1[3]);
//             double h1 = std::sqrt(box1[2] / box1[3]);
//             double w2 = std::sqrt(box2[2] * box2[3]);
//             double h2 = std::sqrt(box2[2] / box2[3]);
//             double dx = (x2 - x1) / time_gap;
//             double dy = (y1 - y2) / time_gap;
//             double dw = (w2 - w1) / time_gap;
//             double dh = (h2 - h1) / time_gap;

//             for (int i = 0; i < time_gap; i++) {
//                 double x = x1 + (i + 1) * dx;
//                 double y = y1 + (i + 1) * dy;
//                 double w = w1 + (i + 1) * dw;
//                 double h = h1 + (i + 1) * dh;
//                 double s = w * h;
//                 double r = w / (h * 1.0);
//                 Eigen::Matrix<float, 4, 1> new_box;
//                 new_box << x, y, s, r;
//                 this->y = new_box - this->H * this->x;
//                 Eigen::Matrix<float, 7, 4> PHT = this->P * this->H.transpose();
//                 this->S = this->H * PHT + this->R;
//                 this->SI = (this->S).inverse();
//                 this->K = PHT * this->SI;
//                 this->x = this->x + this->K * this->y;
//                 Eigen::Matrix<float, 7, 7> I_KH = this->I - this->K * this->H;
//                 this->P = ((I_KH * this->P) * I_KH.transpose()) + ((this->K * this->R) * (this->K).transpose());
//                 this->z = new_box;
//                 this->x_post = this->x;
//                 this->P_post = this->P;
//                 if (i != (time_gap - 1)) predict();
//             }
//         } 
//     }
// }// namespace ocsort


#include "include/KalmanFilter.hpp"
#include <cmath>
#include <iostream>
namespace ocsort {
    KalmanFilterNew::KalmanFilterNew() {};
    KalmanFilterNew::KalmanFilterNew(int dim_x_, int dim_z_) {
        // dim_x_ and dim_z_ are accepted for API compatibility but ignored:
        // matrices are now fixed at 7 and 4 (compile-time).
        (void)dim_x_; (void)dim_z_;
        x.setZero();
        P.setIdentity();
        Q.setIdentity();
        B.setIdentity();
        F.setIdentity();
        H.setZero();
        R.setIdentity();
        M.setZero();
        z.setZero();
        K.setZero();
        y.setZero();
        S.setZero();
        SI.setZero();

        x_prior = x;
        P_prior = P;
        x_post = x;
        P_post = P;
    };
    void KalmanFilterNew::predict() {
        x = F * x;
        P = _alpha_sq * F * P * F.transpose() + Q;
        x_prior = x;
        P_prior = P;
    }
    void KalmanFilterNew::update(Eigen::VectorXf* z_) {
        if (z_ == nullptr) {
            history_obs.push_back(nullptr);
        } else {
            history_obs.push_back(std::make_shared<Eigen::VectorXf>(*z_));
        }
        // Cap history_obs growth: unfreeze() only ever needs the last few non-null
        // entries, and max_age=30 means a tracker is erased long before this matters.
        // 100 gives ample headroom and prevents unbounded memory growth on long videos.
        constexpr size_t HISTORY_OBS_CAP = 100;
        if (history_obs.size() > HISTORY_OBS_CAP) {
            history_obs.erase(history_obs.begin(),
                              history_obs.begin() + (history_obs.size() - HISTORY_OBS_CAP));
        }
        if (z_ == nullptr) {
            if (true == observed) freeze();
            observed = false;
            z.setZero();
            x_post = x;
            P_post = P;
            y.setZero();
            return;
        }
        if (false == observed) unfreeze();
        observed = true;
        // *z_ is dynamic; copy into fixed-size measurement vector.
        Eigen::Matrix<float, 4, 1> z_fixed = *z_;
        y = z_fixed - H * x;
        Eigen::Matrix<float, 7, 4> PHT = P * H.transpose();
        S = H * PHT + R;
        SI = S.inverse();
        K = PHT * SI;
        x = x + K * y;
        Eigen::Matrix<float, 7, 7> I_KH = I - K * H;
        P = ((I_KH * P) * I_KH.transpose()) + ((K * R) * K.transpose());
        z = z_fixed;
        x_post = x;
        P_post = P;
    }
    void KalmanFilterNew::freeze() {
        attr_saved.IsInitialized = true;
        attr_saved.x = x;
        attr_saved.P = P;
        attr_saved.Q = Q;
        attr_saved.B = B;
        attr_saved.F = F;
        attr_saved.H = H;
        attr_saved.R = R;
        attr_saved._alpha_sq = _alpha_sq;
        attr_saved.M = M;
        attr_saved.z = z;
        attr_saved.K = K;
        attr_saved.y = y;
        attr_saved.S = S;
        attr_saved.SI = SI;
        attr_saved.x_prior = x_prior;
        attr_saved.P_prior = P_prior;
        attr_saved.x_post = x_post;
        attr_saved.P_post = P_post;
        attr_saved.history_obs = history_obs;
    }
    void KalmanFilterNew::unfreeze() {
        if (true == attr_saved.IsInitialized) {
            new_history = history_obs;
            x = attr_saved.x;
            P = attr_saved.P;
            Q = attr_saved.Q;
            B = attr_saved.B;
            F = attr_saved.F;
            H = attr_saved.H;
            R = attr_saved.R;
            _alpha_sq = attr_saved._alpha_sq;
            M = attr_saved.M;
            z = attr_saved.z;
            K = attr_saved.K;
            y = attr_saved.y;
            S = attr_saved.S;
            SI = attr_saved.SI;
            x_prior = attr_saved.x_prior;
            P_prior = attr_saved.P_prior;
            x_post = attr_saved.x_post;
            P_post = attr_saved.P_post;
            if (history_obs.empty()) {
                return;
            }
            history_obs.erase(history_obs.end() - 1);
            Eigen::VectorXf box1;           
            Eigen::VectorXf box2;           
            int lastNotNullIndex = -1;     
            int secondLastNotNullIndex = -1;
            for (int i = static_cast<int>(new_history.size()) - 1; i >= 0; i--) {
                if (new_history[i] != nullptr) {
                    if (lastNotNullIndex == -1) {
                        lastNotNullIndex = i;
                        box2 = *(new_history.at(lastNotNullIndex));
                    }
                    else if (secondLastNotNullIndex == -1) {
                        secondLastNotNullIndex = i;
                        box1 = *(new_history.at(secondLastNotNullIndex));
                        break;
                    }
                }
            }
            if (secondLastNotNullIndex == -1 || lastNotNullIndex == -1) {
                return;
            }
            double time_gap = lastNotNullIndex - secondLastNotNullIndex;
            if (time_gap <= 0.0 || box1.size() < 4 || box2.size() < 4) {
                return;
            }
            double x1 = box1[0];
            double x2 = box2[0];
            double y1 = box1[1];
            double y2 = box2[1];
            if (!std::isfinite(x1) || !std::isfinite(x2) ||
                !std::isfinite(y1) || !std::isfinite(y2) ||
                !std::isfinite(static_cast<double>(box1[2])) ||
                !std::isfinite(static_cast<double>(box1[3])) ||
                !std::isfinite(static_cast<double>(box2[2])) ||
                !std::isfinite(static_cast<double>(box2[3])) ||
                box1[2] <= 0.0f || box1[3] <= 0.0f ||
                box2[2] <= 0.0f || box2[3] <= 0.0f) {
                return;
            }
            double w1 = std::sqrt(box1[2] * box1[3]);
            double h1 = std::sqrt(box1[2] / box1[3]);
            double w2 = std::sqrt(box2[2] * box2[3]);
            double h2 = std::sqrt(box2[2] / box2[3]);
            if (!std::isfinite(w1) || !std::isfinite(h1) ||
                !std::isfinite(w2) || !std::isfinite(h2) ||
                h1 <= 0.0 || h2 <= 0.0) {
                return;
            }
            double dx = (x2 - x1) / time_gap;
            double dy = (y1 - y2) / time_gap;
            double dw = (w2 - w1) / time_gap;
            double dh = (h2 - h1) / time_gap;
            for (int i = 0; i < time_gap; i++) {
                double x = x1 + (i + 1) * dx;
                double y = y1 + (i + 1) * dy;
                double w = w1 + (i + 1) * dw;
                double h = h1 + (i + 1) * dh;
                double s = w * h;
                double r = w / (h * 1.0);
                if (!std::isfinite(x) || !std::isfinite(y) ||
                    !std::isfinite(s) || !std::isfinite(r) ||
                    h <= 0.0 || s <= 0.0) {
                    return;
                }
                Eigen::Matrix<float, 4, 1> new_box;
                new_box << x, y, s, r;
                this->y = new_box - this->H * this->x;
                Eigen::Matrix<float, 7, 4> PHT = this->P * this->H.transpose();
                this->S = this->H * PHT + this->R;
                this->SI = (this->S).inverse();
                this->K = PHT * this->SI;
                this->x = this->x + this->K * this->y;
                Eigen::Matrix<float, 7, 7> I_KH = this->I - this->K * this->H;
                this->P = ((I_KH * this->P) * I_KH.transpose()) + ((this->K * this->R) * (this->K).transpose());
                this->z = new_box;
                this->x_post = this->x;
                this->P_post = this->P;
                if (i != (time_gap - 1)) predict();
            }
        } 
    }
}// namespace ocsort


